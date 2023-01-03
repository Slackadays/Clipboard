/*  Clipboard - Cut, copy, and paste anything, anywhere, all from the terminal.
    Copyright (C) 2023 Jackson Huff and other contributors on GitHub.com
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.*/
#include "x11.hpp"
#include "logging.hpp"

#include <chrono>
#include <cstdint>
#include <cstring>
#include <exception>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <ranges>
#include <string_view>
#include <thread>
#include <variant>
#include <vector>
#include <limits>

#include <X11/Xlib.h>

namespace chrono = std::chrono;
namespace ranges = std::ranges;

using namespace std::literals;

constexpr auto maxEventPollTime = 5s;
constexpr auto startEventPollBackoff = 1ms;
constexpr auto eventPollBackoffMultiplier = 2;
constexpr auto maxEventPollBackoffTime = 500ms;

constexpr auto atomClipboard = "CLIPBOARD";
constexpr auto atomTargets = "TARGETS";
constexpr auto atomIncr = "INCR";

// Forward declarations
class X11ClipboardType;
class X11Exception;
class X11Atom;
class X11Connection;
class X11PropertyFormat;
class X11Property;
class X11PropertyIterator;
class X11Window;

enum class X11PropertyMode;

#define X_CALL(name, ...) doXCall(#name, &name, __VA_ARGS__)

template<typename T>
using X11Pointer = std::unique_ptr<T, decltype(&XFree)>;

template<typename T>
X11Pointer<T> capture(T* ptr) {
    return { ptr, &XFree };
}

class X11ClipboardType {
private:
    unsigned int const m_priority;
    char const* const m_name;
    ClipboardContentType const m_type;

public:
    X11ClipboardType(
        unsigned int priority,
        char const* const name,
        ClipboardContentType type)
        : m_priority(priority)
        , m_name(name)
        , m_type(type) { }

    [[nodiscard]] inline unsigned int priority() const { return m_priority; }
    [[nodiscard]] inline char const* const name() const { return m_name; }
    [[nodiscard]] inline ClipboardContentType type() const { return m_type; }

private:
    inline static std::map<std::string_view, X11ClipboardType> s_typesByName {};
    inline static std::vector<std::pair<char const* const, ClipboardContentType>> const s_knownTypes {
        { "x-special/gnome-copied-files", ClipboardContentType::Paths },
        { "text/uri-list", ClipboardContentType::Paths },
        { "text/plain;charset=utf-8", ClipboardContentType::Text },
        { "UTF8_STRING", ClipboardContentType::Text },
        { "text/plain", ClipboardContentType::Text },
        { "STRING", ClipboardContentType::Text },
        { "TEXT", ClipboardContentType::Text },
    };

    static void tryPopulateTypesByName();
public:
    static std::optional<X11ClipboardType> findBest(std::vector<std::reference_wrapper<X11Atom const>> const&);
};

class X11Exception : public std::exception {
private:
    std::variant<std::string, char const*> m_message;
public:
    explicit X11Exception(char const* const message) : m_message(message) {}
    explicit X11Exception(std::string&& message) : m_message(message) {}

    [[nodiscard]] char const* what() const noexcept override {
        return std::visit([](auto&& message) -> char const* {
            using T = std::decay_t<decltype(message)>;
            if constexpr (std::is_same_v<char const*, T>)
                return message;
            else
                return message.c_str();
        }, m_message);
    }
};

class X11Atom {
private:
    Atom m_value;
    std::variant<char const*, X11Pointer<char>> m_name;

public:
    X11Atom(Atom value, X11Pointer<char>&& name) : m_value(value), m_name(std::move(name)) { }
    X11Atom(Atom value, char const* name) : m_value(value), m_name(name) { }

    [[nodiscard]] inline Atom value() const { return m_value; }
    [[nodiscard]] std::string_view name() const;

    bool operator==(X11Atom const& other) const;
};

class X11Connection {
public:
    X11Connection(X11Connection const&) = delete;
    X11Connection& operator=(X11Connection const&) = delete;

    X11Connection(X11Connection&&) = delete;
    X11Connection& operator=(X11Connection&&) = delete;

private:
    inline static X11Connection* instance = nullptr;
    static int globalErrorHandler(Display*, XErrorEvent*);

    Display* m_display;
    std::map<std::string_view const, std::shared_ptr<X11Atom>> m_atoms_by_name;
    std::map<Atom const, std::shared_ptr<X11Atom>> m_atoms_by_value;

    std::optional<std::string_view> m_currentXCall;
    std::optional<X11Exception> m_pendingXCallException;

    int localErrorHandler(Display*, XErrorEvent*);
    void throwIfDestroyed() const;
    X11Atom const& addAtomToCache(X11Atom&&);

public:
    explicit X11Connection();
    ~X11Connection();

    template<typename F, typename... Args>
    inline auto doXCall(std::string_view callName, F callLambda, Args... args);

    [[nodiscard]] inline Display* display() const { return m_display; }

    X11Atom const& atom(char const*);
    X11Atom const& atom(Atom);

    Window getSelectionOwner(X11Atom const&);
    bool isClipboardOwned();

    X11Window createWindow();
};

class X11PropertyFormat {
private:
    constexpr explicit X11PropertyFormat(std::size_t value, std::size_t size)
        : m_value(value), m_size(size) { }

    std::size_t m_value;
    std::size_t m_size;

public:
    enum Value : std::size_t {
        Format8 = 8,
        Format16 = 16,
        Format32 = 32
    };

    constexpr X11PropertyFormat(Value value) : X11PropertyFormat(
        static_cast<std::size_t>(value),
        value == Format8 ? sizeof(std::uint8_t) :
        value == Format8 ? sizeof(std::uint16_t) :
        sizeof(std::uint64_t) // No, this is not a mistake. libx11 uses a 64-bit int for the 32 format
    ) { }

    [[nodiscard]] inline std::size_t size() const { return m_size; }
    [[nodiscard]] inline std::size_t value() const { return m_value; }

    bool operator<=>(X11PropertyFormat const&) const = default;

    static X11PropertyFormat fromValue(std::size_t value) {
        return { static_cast<Value>(value) };
    }
};

enum class X11PropertyMode : int {
    Replace = PropModeReplace,
    Append = PropModeAppend,
    Prepend = PropModePrepend
};

class X11Property {
private:
    X11Atom const& m_name;
    X11Atom const& m_type;
    X11PropertyFormat const m_format;

    std::variant<std::uint8_t const*, std::unique_ptr<std::uint8_t const[]>> const m_data8;
    std::size_t const m_size8;

public:
    X11Property(X11Atom const& name, X11Atom const& type, std::u8string_view data)
        : m_name(name)
        , m_type(type)
        , m_format(X11PropertyFormat::Format8)
        , m_data8(reinterpret_cast<uint8_t const*>(data.data()))
        , m_size8(data.size()) {}

    X11Property(
            X11Atom const& name,
            X11Atom const& type,
            X11PropertyFormat format,
            std::unique_ptr<std::uint8_t[]>&& data8,
            std::size_t size8)
        : m_name(name)
        , m_type(type)
        , m_format(format)
        , m_data8(std::move(data8))
        , m_size8(size8) {}

    [[nodiscard]] inline X11Atom const& name() const { return m_name; }
    [[nodiscard]] inline X11Atom const& type() const { return m_type; }
    [[nodiscard]] inline X11PropertyFormat format() const { return m_format; }

    [[nodiscard]] std::size_t size() const { return m_size8 / m_format.size(); }

    [[nodiscard]] std::size_t size8() const { return m_size8; }
    [[nodiscard]] std::size_t size16() const { return size8() / X11PropertyFormat{X11PropertyFormat::Format16}.size(); }
    [[nodiscard]] std::size_t size32() const { return size8() / X11PropertyFormat{X11PropertyFormat::Format32}.size(); }

    [[nodiscard]] std::uint8_t const* data8() const {
        return std::visit([](auto&& arg) -> std::uint8_t const* {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, uint8_t const*>)
                return arg;
            else
                return arg.get();
        }, m_data8);
    }

    [[nodiscard]] std::uint16_t const* data16() const { return reinterpret_cast<std::uint16_t const*>(data8()); }
    [[nodiscard]] std::uint64_t const* data32() const { return reinterpret_cast<std::uint64_t const*>(data8()); }

    [[nodiscard]] X11PropertyIterator begin() const;
    [[nodiscard]] X11PropertyIterator end() const;
};

class X11PropertyIterator {
private:
    X11Property const& m_property;
    std::size_t m_offset;

public:
    X11PropertyIterator(X11Property const& property, std::size_t offset) : m_property(property), m_offset(offset) { }

    std::uint64_t operator*() const;
    X11PropertyIterator& operator++();
    std::partial_ordering operator<=>(X11PropertyIterator const&) const;

    operator bool() const;
};

class X11Window {
public:
    X11Window(X11Window const&) = delete;
    X11Window& operator=(X11Window const&) = delete;

private:
    X11Connection&  m_connection;
    Window m_window;

    void throwIfDestroyed() const;
public:
    X11Window(X11Connection&, Window);
    ~X11Window();

    [[nodiscard]] inline X11Connection& connection() const { return m_connection; }
    [[nodiscard]] inline Display* display() const { return m_connection.display(); }
    [[nodiscard]] inline Window window() const { return m_window; }

    [[nodiscard]] inline X11Atom const& atom(char const* name) const { return m_connection.atom(name); }
    [[nodiscard]] inline X11Atom const& atom(Atom value) const { return m_connection.atom(value); }

    [[nodiscard]] Time queryCurrentTime();
    [[nodiscard]] std::optional<XEvent> checkTypedWindowEvent(int eventType);
    [[nodiscard]] std::optional<XEvent> checkMaskEvent(int eventMask);
    void changeProperty(X11PropertyMode, X11Property&);
    void deleteProperty(X11Atom const&);
    [[nodiscard]] X11Property getProperty(X11Atom const&, bool delet = false);
    [[nodiscard]] X11Property convertSelection(X11Atom const& selection, X11Atom const& target);

    [[nodiscard]] std::vector<std::reference_wrapper<X11Atom const>> queryClipboardTargets();
    [[nodiscard]] X11Property convertClipboard(X11Atom const& target);
    [[nodiscard]] std::vector<char> getClipboardData(X11Atom const& target);

    template<typename predicate_t>
    XEvent waitForEvent(int eventType, predicate_t predicate);

    template<typename F, typename... Args>
    inline auto doXCall(std::string_view callName, F callLambda, Args... args) {
        return connection().doXCall(callName, callLambda, args...);
    }
};

void X11ClipboardType::tryPopulateTypesByName() {
    if (!s_typesByName.empty()) {
        return;
    }

    unsigned int priority = 0;
    for (auto&& [ name, type ] : s_knownTypes) {
        s_typesByName.insert({ name, { priority++, name, type } });
    }
}

std::optional<X11ClipboardType> X11ClipboardType::findBest(std::vector<std::reference_wrapper<const X11Atom>> const& targets) {
    tryPopulateTypesByName();

    std::optional<X11ClipboardType> best {};
    for (auto&& target : targets) {
        debugStream << "Advertised target: " << target.get().name() << std::endl;

        auto&& it = s_typesByName.find(target.get().name());
        if (it == s_typesByName.end()) {
            continue;
        }

        X11ClipboardType found = it->second;
        if (best.has_value() && best->priority() <= found.priority()) {
            continue;
        }

        best.emplace(found);
    }

    return best;
}

std::string_view X11Atom::name() const {
    return std::visit([](auto&& name) -> char const* {
        using T = std::decay_t<decltype(name)>;
        if constexpr (std::is_same_v<char const*, T>) {
            return name;
        } else {
            return name.get();
        }
    }, m_name);
}

bool X11Atom::operator==(const X11Atom &other) const {
    return m_value == other.m_value;
}

int X11Connection::globalErrorHandler(Display* const display, XErrorEvent* const event) {
    if (instance != nullptr) {
        return instance->localErrorHandler(display, event);
    }

    debugStream << "Global error handler called but no X11 connection is active" << std::endl;
    return 0;
}

int X11Connection::localErrorHandler(Display* const errorDisplay, XErrorEvent* const event) {
    throwIfDestroyed();

    std::stringstream message;
    if (m_currentXCall.has_value()) {
        message << m_currentXCall.value();
    } else {
        message << "X11";
    }

    message << ": ";

    if (event != nullptr) {
        char xmessageBuffer[1024] = { 0 };
        XGetErrorText(display(), event->error_code, xmessageBuffer, 1024);

        message << xmessageBuffer;
    }

    debugStream << "Error during X11 call: " << message.str() << std::endl;
    m_pendingXCallException.emplace(message.str());
    return 0;
}

template<typename F, typename... Args>
inline auto X11Connection::doXCall(std::string_view callName, F callLambda, Args... args) {
    throwIfDestroyed();

    if (m_currentXCall.has_value()) {
        std::stringstream message;
        message << "Tried to call " << callName << " while a call to " << m_currentXCall.value() << " was already in progress";
        throw X11Exception(message.str());
    }

    m_currentXCall.emplace(callName);
    m_pendingXCallException.reset();

    auto result = callLambda(args...);

    m_currentXCall.reset();
    if (m_pendingXCallException.has_value()) {
        throw m_pendingXCallException.value();
    }

    return result;
}

X11Window X11Connection::createWindow() {
    throwIfDestroyed();

    XSetWindowAttributes attributes {
        .event_mask = PropertyChangeMask
    };

    auto handle = XCreateWindow(
        /*display*/ display(),
        /*parent*/ DefaultRootWindow(display()),
        /*x*/ -10,
        /*y*/ -10,
        /*width*/ 1,
        /*height*/ 1,
        /*border_width*/ 0,
        /*depth*/ CopyFromParent,
        /*class*/ InputOutput,
        /*visual*/ CopyFromParent,
        /*valuemask*/ CWEventMask,
        /*attributes*/ &attributes
    );
    return { *this, handle };
}

X11Connection::X11Connection() {
    debugStream << "Opening X11 connection" << std::endl;

    XSetErrorHandler(&globalErrorHandler);

    if (instance != nullptr && instance != this) {
        throw X11Exception("Only one X11 connection can be open at a time");
    }

    m_display = XOpenDisplay(nullptr);
    if (m_display == nullptr) {
        std::stringstream message;
        message << "XOpenDisplay: failed to open display " << XDisplayName(nullptr);
        throw X11Exception(message.str());
    }

    instance = this;
}

X11Connection::~X11Connection() {
    debugStream << "Closing X11 connection" << std::endl;

    X_CALL(XCloseDisplay, m_display);
    m_display = nullptr;
    instance = nullptr;
}

X11Atom const& X11Connection::addAtomToCache(X11Atom&& atom) {
    auto ptr = std::make_shared<X11Atom>(std::move(atom));
    m_atoms_by_name.insert({ ptr->name(), ptr });
    m_atoms_by_value.insert({ ptr->value(), ptr });
    return *ptr;
}

X11Atom const& X11Connection::atom(const char* const name) {
    throwIfDestroyed();

    if (m_atoms_by_name.contains(name)) {
        return *m_atoms_by_name.at(name);
    }

    auto const value = X_CALL(XInternAtom, display(), name, false);
    if (value == None) {
        throw X11Exception("Unable to intern value");
    }

    return addAtomToCache({ value, name });
}

X11Atom const& X11Connection::atom(Atom value) {
    throwIfDestroyed();

    if (m_atoms_by_value.contains(value)) {
        return *m_atoms_by_value.at(value);
    }

    auto name = X_CALL(XGetAtomName, display(), value);
    if (name == nullptr) {
        throw X11Exception("Unable to get atom name");
    }

    return addAtomToCache({ value, capture(name) });
}

void X11Connection::throwIfDestroyed() const {
    if (m_display == nullptr) {
        throw X11Exception("Tried to use a connection after it was destroyed");
    }
}


Window X11Connection::getSelectionOwner(const X11Atom& selection) {
    throwIfDestroyed();
    return X_CALL(XGetSelectionOwner, display(), selection.value());
}

bool X11Connection::isClipboardOwned() {
    throwIfDestroyed();
    return getSelectionOwner(atom(atomClipboard)) != None;
}

X11PropertyIterator X11Property::begin() const {
    return { *this, 0 };
}

X11PropertyIterator X11Property::end() const {
    return { *this, size() };
}

std::partial_ordering X11PropertyIterator::operator<=>(X11PropertyIterator const& other) const {
    if (std::addressof(other.m_property) != std::addressof(m_property)) {
        return std::partial_ordering::unordered;
    }

    return m_offset <=> other.m_offset;
}

std::uint64_t X11PropertyIterator::operator*() const {
    auto pointer8 = m_property.data8() + (m_property.format().size() * m_offset);

    if (m_property.format() == X11PropertyFormat::Format8) {
        return *pointer8;
    }

    if (m_property.format() == X11PropertyFormat::Format16) {
        return *reinterpret_cast<std::uint16_t const*>(pointer8);
    }

    if (m_property.format() == X11PropertyFormat::Format32) {
        return *reinterpret_cast<std::uint64_t const*>(pointer8);
    }

    throw X11Exception("Unknown property format");
}

X11PropertyIterator& X11PropertyIterator::operator++() {
    m_offset++;
    return *this;
}

X11PropertyIterator::operator bool() const {
    return m_offset < m_property.size();
}

X11Window::X11Window(X11Connection& connection, Window window)
        : m_connection(connection)
        , m_window(window) {

    if (m_window == None) {
        throw X11Exception("Invalid Window");
    }
}

X11Window::~X11Window() {
    XDestroyWindow(display(), m_window);
    m_window = None;
}

void X11Window::throwIfDestroyed() const {
    if (m_window == None) {
        throw X11Exception("Tried to use a connection after it was destroyed");
    }
}

std::optional<XEvent> X11Window::checkTypedWindowEvent(int eventType) {
    throwIfDestroyed();

    XEvent event;
    if (X_CALL(XCheckTypedWindowEvent, display(), window(), eventType, &event) == True) {
        return event;
    }
    return {};
}

std::optional<XEvent> X11Window::checkMaskEvent(int eventMask) {
    throwIfDestroyed();

    XEvent event;
    if (X_CALL(XCheckMaskEvent, display(), eventMask, &event) == True) {
        return event;
    }
    return {};
}

void X11Window::changeProperty(X11PropertyMode mode, X11Property& value) {
    throwIfDestroyed();
    X_CALL(XChangeProperty,
        display(),
        window(),
        value.name().value(),
        value.type().value(),
        value.format().value(),
        static_cast<int>(mode),
        value.data8(),
        value.size()
    );
}

void X11Window::deleteProperty(X11Atom const& property) {
    throwIfDestroyed();
    X_CALL(XDeleteProperty, display(), window(), property.value());
}

template<typename predicate_t>
XEvent X11Window::waitForEvent(int eventType, predicate_t predicate) {
    throwIfDestroyed();

    debugStream << "Waiting for event " << eventType << std::endl;

    auto const startTime = chrono::steady_clock::now();
    auto backoffTime = startEventPollBackoff;

    std::optional<XEvent> event;
    while (
        !(event = checkTypedWindowEvent(eventType)).has_value()
        || !predicate(event.value())
    ) {
        if (event.has_value()) {
            debugStream << "Got an event but it didn't match the predicate, sleeping" << std::endl;
        } else {
            debugStream << "No events, sleeping" << std::endl;
        }

        auto const time = chrono::steady_clock::now() - startTime;
        if (time >= maxEventPollTime) {
            debugStream << "Timeout waiting for event" << std::endl;
            throw X11Exception("Timed-out waiting for an event");
        }

        std::this_thread::sleep_for(backoffTime);
        backoffTime = eventPollBackoffMultiplier * backoffTime;
        if (backoffTime > maxEventPollBackoffTime) {
            backoffTime = maxEventPollBackoffTime;
        }
    }

    debugStream << "Got the event we were waiting for" << std::endl;
    return event.value();
}

Time X11Window::queryCurrentTime() {
    throwIfDestroyed();

    auto&& name = atom("GETCURRENTTIME");
    X11Property value { name, atom("text/plain"), u8"getcurrenttime"sv };

    deleteProperty(name);
    changeProperty(X11PropertyMode::Replace, value);

    auto const event = waitForEvent(PropertyNotify, [&name](XEvent& event) {
        return event.xproperty.atom == name.value() && event.xproperty.state == PropertyNewValue;
    });

    deleteProperty(name);
    return event.xproperty.time;
}

X11Property X11Window::convertSelection(X11Atom const& selection, X11Atom const& target) {
    throwIfDestroyed();

    auto&& property = atom("convertSelectionProperty");
    auto const requestor = window();

    deleteProperty(property);
    X_CALL(XConvertSelection,
        display(),
        selection.value(),
        target.value(),
        property.value(),
        requestor,
        queryCurrentTime()
    );

    auto const result = waitForEvent(SelectionNotify, [requestor, &selection, &target](XEvent& event) {
        auto& xselection = event.xselection;
        return xselection.requestor == requestor
               && xselection.selection == selection.value()
               && xselection.target == target.value();
    });

    if (result.xselection.property == None) {
        throw X11Exception("Selection owner refused selection request");
    }

    return getProperty(property, true);
}

X11Property X11Window::getProperty(X11Atom const& name, bool delet) {
    throwIfDestroyed();

    Atom actualTypeReturn = None;
    int actualFormatReturn = 0;
    std::size_t nitemsReturn = 0;
    std::size_t bytesAfterReturn = 0;
    std::uint8_t* propReturn = nullptr;

    X_CALL(XGetWindowProperty,
       /*display*/ display(),
       /*w*/ window(),
       /*property*/ name.value(),
       /*long_offset*/ 0,
       /*long_length*/ std::numeric_limits<uint32_t>::max(),
       /*delete*/ delet,
       /*req_type*/ AnyPropertyType,
       /*actual_type_return*/ &actualTypeReturn,
       /*actual_format_return*/ &actualFormatReturn,
       /*nitems_return*/ &nitemsReturn,
       /*bytes_after_return*/ &bytesAfterReturn,
       /*prop_return*/ &propReturn
    );

    // Capture before doing any checks to ensure data will be freed no matter what happens
    auto x11Data = capture(propReturn);

    if (bytesAfterReturn > 0) {
        std::stringstream message;
        message << "XGetWindowProperty read " << nitemsReturn << " items but left " << bytesAfterReturn << " bytes behind";
        throw X11Exception(message.str());
    }

    auto&& type = atom(actualTypeReturn);
    auto const format = X11PropertyFormat::fromValue(actualFormatReturn);

    auto const size = nitemsReturn * format.size();
    auto data = std::make_unique<std::uint8_t[]>(size);
    std::memcpy(data.get(), x11Data.get(), size);

    return X11Property {
        name,
        type,
        format,
        std::move(data),
        size
    };
}

X11Property X11Window::convertClipboard(const X11Atom &target) {
    return convertSelection(atom(atomClipboard), target);
}

std::vector<std::reference_wrapper<X11Atom const>> X11Window::queryClipboardTargets() {
    throwIfDestroyed();

    auto property = convertClipboard(atom(atomTargets));

    std::vector<std::reference_wrapper<X11Atom const>> result{};
    result.reserve(property.size());

    for (auto&& atomValue : property) {
        result.emplace_back(atom(atomValue));
    }

    return result;
}

std::vector<char> X11Window::getClipboardData(X11Atom const& target) {
    throwIfDestroyed();

    std::vector<char> result;
    auto addToResult = [&result](X11Property const& x) {
        for (auto&& c : x) {
            result.push_back(c);
        }
    };

    auto firstResult = convertClipboard(target);
    if (firstResult.type() != atom(atomIncr)) {
        debugStream << "Got a regular non-INCR result" << std::endl;
        addToResult(firstResult);
        return std::move(result);
    }

    debugStream << "Got an INCR result" << std::endl;

    // The value of INCR should be a lower bound on the size of the full data
    result.reserve(*firstResult.begin());

    while (true) {
        waitForEvent(PropertyNotify, [&](XEvent const& event) {
            return event.xproperty.atom == firstResult.name().value()
                && event.xproperty.state == PropertyNewValue;
        });
        auto prop = getProperty(firstResult.name(), true);
        if (prop.size() <= 0) {
            break;
        }

        debugStream << "Got a chunk of " << prop.size() << " bytes" << std::endl;
        addToResult(prop);
    }

    debugStream << "DONE! Received a total of " << result.size() << " bytes" << std::endl;
    return result;
}

static std::string urlDecode(std::string const& value) {
    auto tryConvertByte = [](std::string const& str) -> std::optional<char> {
        std::size_t pos = 0;
        unsigned long result;
        try {
            result = std::stoul(str, &pos, 16);
        } catch (std::invalid_argument&) {
            return {};
        }

        if (pos != 2) {
            return {};
        }

        return static_cast<char>(result);
    };

    std::vector<char> result;

    for (std::size_t i = 0; i < value.size(); i++) {
        if (value[i] != '%' || i >= value.size() - 2) {
            result.push_back(value[i]);
            continue;
        }

        auto possibleByte = tryConvertByte({ &value[i + 1], 2 });
        if (possibleByte.has_value()) {
            result.push_back(possibleByte.value());
            i += 2;
        } else {
            result.push_back('%');
        }
    }

    return { result.begin(), result.end() };
}

static std::string parseText(std::vector<char> const& data) {
    return { data.begin(), data.end() };
}

static ClipboardPaths parseFiles(std::vector<char> const& data) {
    std::string dataStr { data.begin(), data.end() };
    std::istringstream stream { dataStr };

    ClipboardPathsAction action = ClipboardPathsAction::Copy;
    std::vector<fs::path> paths {};
    while (!stream.eof()) {
        std::string line;
        std::getline(stream, line);

        if (line.empty()) {
            continue;
        }

        if (line == "copy"sv) {
            action = ClipboardPathsAction::Copy;
            debugStream << "Action: copy" << std::endl;
            continue;
        }

        if (line == "cut"sv) {
            action = ClipboardPathsAction::Cut;
            debugStream << "Action: cut" << std::endl;
            continue;
        }

        if (line.starts_with("file://"sv)) {
            line.erase(0, "file://"sv.size());
            line = urlDecode(line);
        }

        debugStream << "file: " << line << std::endl;
        paths.emplace_back(line);
    }

    return { action, std::move(paths) };
}

static ClipboardContent getX11ClipboardInternal() {
    X11Connection conn;
    if (!conn.isClipboardOwned()) {
        debugStream << "No selection owner, aborting" << std::endl;
        return {};
    }

    auto window = conn.createWindow();
    auto target = X11ClipboardType::findBest(window.queryClipboardTargets());

    if (!target) {
        debugStream << "No supported target was advertised, aborting" << std::endl;
        return {};
    }

    debugStream << "Chosen target: " << target->name() << std::endl;
    auto const data = window.getClipboardData(conn.atom(target->name()));

    if (target->type() == ClipboardContentType::Text) {
        return { parseText(data) };
    }

    return { parseFiles(data) };
}

ClipboardContent getX11Clipboard() {
    try {
        return getX11ClipboardInternal();
    } catch (X11Exception const& e) {
        debugStream << "Error getting data from X11: " << e.what() << std::endl;
        return {};
    }
}
