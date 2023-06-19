/*  The Clipboard Project - Cut, copy, and paste anything, anytime, anywhere, all from the terminal.
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
#include <chrono>
#include <cstdint>
#include <cstring>
#include <exception>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <ranges>
#include <set>
#include <string_view>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "clipboard/x11wl/mime.hpp"
#include <X11/Xlib.h>
#include <clipboard/gui.hpp>
#include <clipboard/logging.hpp>
#include <clipboard/utils.hpp>

namespace chrono = std::chrono;
namespace ranges = std::ranges;
namespace views = std::views;

using namespace std::literals;

constexpr auto atomClipboard = "CLIPBOARD";
constexpr auto atomTargets = "TARGETS";
constexpr auto atomMultiple = "MULTIPLE";
constexpr auto atomTimestamp = "TIMESTAMP";
constexpr auto atomIncr = "INCR";
constexpr auto atomAtom = "ATOM";
constexpr auto atomAtomPair = "ATOM_PAIR";
constexpr auto atomInteger = "INTEGER";

// Forward declarations
class X11Exception;
class X11Atom;
class X11Connection;
class X11PropertyFormat;
class X11Property;
class X11PropertyIterator;
class X11Window;
class X11SelectionRequest;
class X11SelectionTransfer;
class X11IncrTransfer;
class X11SelectionDaemon;

enum class X11PropertyMode;

#define X_CALL(name, ...) doXCall(#name, &name, __VA_ARGS__)

template <typename T>
using X11Pointer = std::unique_ptr<T, decltype(&XFree)>;

template <typename T>
X11Pointer<T> capture(T* ptr) {
    return {ptr, &XFree};
}

class X11Exception : public SimpleException {
    using SimpleException::SimpleException;

public:
    unsigned char m_errorCode = 0;
    unsigned char errorCode() const { return m_errorCode; }
};

class X11Atom {
private:
    Atom m_value;
    std::string m_name;

public:
    X11Atom(Atom value, std::string&& name) : m_value(value), m_name(std::move(name)) {}

    [[nodiscard]] inline Atom value() const { return m_value; }
    [[nodiscard]] inline std::string_view name() const { return m_name; }

    bool operator==(const X11Atom& other) const;
    bool operator==(const Atom& other) const;
    bool operator==(const std::string_view& other) const;
};

class X11Connection {
public:
    X11Connection(const X11Connection&) = delete;
    X11Connection& operator=(const X11Connection&) = delete;

    X11Connection(X11Connection&&) = delete;
    X11Connection& operator=(X11Connection&&) = delete;

private:
    inline static X11Connection* instance = nullptr;
    static int globalErrorHandler(Display*, XErrorEvent*);

    Display* m_display;
    std::map<const std::string_view, std::shared_ptr<X11Atom>> m_atoms_by_name;
    std::map<const Atom, std::shared_ptr<X11Atom>> m_atoms_by_value;
    std::map<Window, std::weak_ptr<X11Window>> m_externalWindows;

    std::optional<std::string_view> m_currentXCall;
    std::optional<X11Exception> m_pendingXCallException;

    int localErrorHandler(Display*, XErrorEvent*);
    void throwIfDestroyed() const;
    const X11Atom& addAtomToCache(X11Atom&&);

public:
    explicit X11Connection();
    ~X11Connection();

    template <typename F, typename... Args>
    inline auto doXCall(std::string_view callName, F callLambda, Args... args);

    [[nodiscard]] inline Display* display() const { return m_display; }
    [[nodiscard]] inline std::size_t maxRequestSize() const { return XMaxRequestSize(display()); }
    [[nodiscard]] inline std::size_t maxDataSizeForIncr() const { return maxRequestSize() / 2; }

    const X11Atom& atom(std::string_view);
    const X11Atom& atom(Atom);

    [[nodiscard]] XEvent nextEvent();
    [[nodiscard]] std::optional<XEvent> checkMaskEvent(int eventMask);
    Window getSelectionOwner(const X11Atom&);
    void sendEvent(Window, bool propagate, long eventMask, XEvent& event);
    bool isClipboardOwned();

    X11Window createWindow();
    std::shared_ptr<X11Window> externalWindow(Window);
};

class X11PropertyFormat {
private:
    constexpr explicit X11PropertyFormat(std::size_t value, std::size_t size) : m_value(value), m_size(size) {}

    std::size_t m_value;
    std::size_t m_size;

public:
    using format8_t = std::uint8_t;
    using format16_t = std::uint16_t;
    using format32_t = std::conditional<sizeof(void*) == sizeof(std::uint64_t), std::uint64_t, std::uint32_t>::type;

    enum Value : std::size_t { Format8 = 8, Format16 = 16, Format32 = 32 };

    constexpr X11PropertyFormat(Value value) : X11PropertyFormat(static_cast<std::size_t>(value), value == Format8 ? sizeof(format8_t) : value == Format16 ? sizeof(format16_t) : sizeof(format32_t)) {}

    [[nodiscard]] inline std::size_t size() const { return m_size; }
    [[nodiscard]] inline std::size_t value() const { return m_value; }

    bool operator<=>(const X11PropertyFormat&) const = default;

    static X11PropertyFormat fromValue(std::size_t value) { return {static_cast<Value>(value)}; }

    template <std::size_t char_t>
    constexpr static inline X11PropertyFormat fromSize();
};

enum class X11PropertyMode : int { Replace = PropModeReplace, Append = PropModeAppend, Prepend = PropModePrepend };

class X11Property {
private:
    const X11Atom& m_name;
    const X11Atom& m_type;
    X11PropertyFormat m_format;

    std::variant<const X11PropertyFormat::format8_t*, std::unique_ptr<X11PropertyFormat::format8_t[]>> m_data8;
    std::size_t m_size8;

public:
    template <ranges::contiguous_range range_t, typename char_t = ranges::range_value_t<range_t>>
    X11Property(const X11Atom& name, const X11Atom& type, range_t data, bool owned);

    template <ranges::contiguous_range range_t, typename char_t = ranges::range_value_t<range_t>>
    X11Property(const X11Atom& name, const X11Atom& type, const X11PropertyFormat&, range_t data, bool owned);

    [[nodiscard]] inline const X11Atom& name() const { return m_name; }
    [[nodiscard]] inline const X11Atom& type() const { return m_type; }
    [[nodiscard]] inline X11PropertyFormat format() const { return m_format; }

    [[nodiscard]] std::size_t size() const { return m_size8 / m_format.size(); }

    [[nodiscard]] std::size_t size8() const { return m_size8; }
    [[nodiscard]] std::size_t size16() const { return size8() / sizeof(X11PropertyFormat::format16_t); }
    [[nodiscard]] std::size_t size32() const { return size8() / sizeof(X11PropertyFormat::format32_t); }

    [[nodiscard]] const X11PropertyFormat::format8_t* data8() const {
        return std::visit(
                [](auto&& arg) -> const X11PropertyFormat::format8_t* {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, X11PropertyFormat::format8_t const*>)
                        return arg;
                    else
                        return arg.get();
                },
                m_data8
        );
    }

    [[nodiscard]] const X11PropertyFormat::format16_t* data16() const { return reinterpret_cast<const X11PropertyFormat::format16_t*>(data8()); }
    [[nodiscard]] const X11PropertyFormat::format32_t* data32() const { return reinterpret_cast<const X11PropertyFormat::format32_t*>(data8()); }

    [[nodiscard]] X11PropertyIterator begin() const;
    [[nodiscard]] X11PropertyIterator end() const;

    [[nodiscard]] X11Property range(std::size_t start, std::size_t end);
};

class X11PropertyIterator {
private:
    const X11Property& m_property;
    std::size_t m_offset;

public:
    X11PropertyIterator(const X11Property& property, std::size_t offset) : m_property(property), m_offset(offset) {}

    X11PropertyFormat::format32_t operator*() const;
    X11PropertyIterator& operator++();
    std::partial_ordering operator<=>(const X11PropertyIterator&) const;

    operator bool() const;
};

class X11Window {
public:
    X11Window(const X11Window&) = delete;
    X11Window& operator=(const X11Window&) = delete;

    X11Window(X11Window&&) = default;
    X11Window& operator=(X11Window&&) = default;

private:
    X11Connection& m_connection;
    Window m_window;
    bool m_owned;

    void throwIfDestroyed() const;

public:
    X11Window(X11Connection&, Window);
    X11Window(X11Connection&, Window, bool owned);
    ~X11Window() noexcept(false);

    [[nodiscard]] inline X11Connection& connection() const { return m_connection; }
    [[nodiscard]] inline Display* display() const { return m_connection.display(); }
    [[nodiscard]] inline Window window() const { return m_window; }

    [[nodiscard]] inline const X11Atom& atom(const char* name) const { return m_connection.atom(name); }
    [[nodiscard]] inline const X11Atom& atom(Atom value) const { return m_connection.atom(value); }

    [[nodiscard]] Time queryCurrentTime();
    [[nodiscard]] std::optional<XEvent> checkTypedWindowEvent(int eventType);
    [[nodiscard]] std::optional<XEvent> checkMaskEvent(int eventMask);
    void changeProperty(X11PropertyMode, const X11Property&);
    void deleteProperty(const X11Atom&);
    [[nodiscard]] X11Property getProperty(const X11Atom&, bool delet = false);
    [[nodiscard]] X11Property convertSelection(const X11Atom& selection, const X11Atom& target);
    void setSelectionOwner(const X11Atom& selection, Time time);
    XWindowAttributes getWindowAttributes();
    void changeWindowAttributes(unsigned long valuemask, XSetWindowAttributes* attributes);
    void sendEvent(bool propagate, long eventMask, XEvent& event);

    [[nodiscard]] std::vector<std::reference_wrapper<const X11Atom>> queryClipboardTargets();
    [[nodiscard]] X11Property convertClipboard(const X11Atom& target);
    [[nodiscard]] std::vector<char> getClipboardData(const X11Atom& target);
    void setEventMask(long);
    void addToEventMask(long);
    void addPropertyChangeToEventMask();
    void clearEventMask();

    template <std::predicate<const XEvent&> predicate_t>
    XEvent waitForEvent(int eventType, predicate_t predicate);

    template <typename F, typename... Args>
    inline auto doXCall(std::string_view callName, F callLambda, Args... args) {
        return connection().doXCall(callName, callLambda, args...);
    }

    bool operator==(const X11Window&) const;
    bool operator==(const Window&) const;
};

class X11SelectionRequest {
private:
    XSelectionRequestEvent m_event;
    std::shared_ptr<X11Window> m_window;
    const X11Atom& m_target;
    const X11Atom& m_property;
    bool m_multiple;

    X11SelectionRequest(XSelectionRequestEvent, std::shared_ptr<X11Window>, const X11Atom& target, const X11Atom& property, bool multiple);

public:
    X11SelectionRequest(X11Connection&, XSelectionRequestEvent);

    [[nodiscard]] inline const XSelectionRequestEvent& event() const { return m_event; }
    [[nodiscard]] inline X11Connection& connection() const { return m_window->connection(); }
    [[nodiscard]] inline X11Window& window() const { return *m_window; }
    [[nodiscard]] inline std::shared_ptr<X11Window> windowPtr() const { return m_window; }
    [[nodiscard]] inline const X11Atom& target() const { return m_target; }
    [[nodiscard]] inline const X11Atom& property() const { return m_property; }
    [[nodiscard]] inline bool isMultiple() const { return m_multiple; }

    X11SelectionRequest forMultiple(const X11Atom& target, const X11Atom& property) const;
};

class X11SelectionTransfer {
protected:
    bool m_done = false;

public:
    virtual ~X11SelectionTransfer() {};

    [[nodiscard]] inline bool isDone() const { return m_done; }
    virtual void handle(const XEvent&) = 0;
};

class X11IncrTransfer : public X11SelectionTransfer {
private:
    std::shared_ptr<X11Window> m_window;
    X11Property m_property;
    std::size_t m_offset;
    bool m_sentTrailer = false;

    [[nodiscard]] inline X11Connection& connection() const { return m_window->connection(); }
    [[nodiscard]] inline std::size_t chunkSize() const { return connection().maxDataSizeForIncr() / m_property.format().size(); }

public:
    X11IncrTransfer(std::shared_ptr<X11Window>, X11Property&&);
    void handle(const XEvent&) override;
};

class X11SelectionDaemon {
private:
    X11Connection& m_connection;
    const X11Atom& m_selection;
    const ClipboardContent& m_content;

    X11Window m_window;
    Time m_selectionAcquiredTime;
    bool m_isSelectionOwner;

    std::vector<std::unique_ptr<X11SelectionTransfer>> m_transfers;

    XEvent nextEvent();
    static XEvent makeSelectionNotify(const XSelectionRequestEvent&);
    void refuseSelectionRequest(const XSelectionRequestEvent&) const;
    bool refuseSelectionRequest(const X11SelectionRequest&) const;

    template <ranges::contiguous_range range_t>
    bool replySelectionRequest(const X11SelectionRequest&, const X11Atom& type, range_t);

    void handle(const XEvent&);
    void handleSelectionClear(const XSelectionClearEvent&);
    void handleSelectionRequest(const XSelectionRequestEvent&);
    bool handleSelectionRequest(const X11SelectionRequest&);

    bool handleMultipleSelectionRequest(const X11SelectionRequest&);
    bool handleTimestampSelectionRequest(const X11SelectionRequest&);
    bool handleTargetsSelectionRequest(const X11SelectionRequest&);
    bool handleRegularSelectionRequest(const X11SelectionRequest&);

public:
    explicit X11SelectionDaemon(X11Connection&, const X11Atom& s1election, const ClipboardContent&);

    [[nodiscard]] inline X11Connection& connection() const { return m_connection; }
    [[nodiscard]] inline const X11Atom& selection() const { return m_selection; }
    [[nodiscard]] inline X11Window& window() { return m_window; }
    [[nodiscard]] inline const ClipboardContent& content() const { return m_content; }
    [[nodiscard]] inline bool isSelectionOwner() const { return m_isSelectionOwner; }

    [[nodiscard]] inline const X11Atom& atom(std::string_view name) const { return m_connection.atom(name); }
    [[nodiscard]] inline const X11Atom& atom(Atom value) const { return m_connection.atom(value); }

    void run();
};

bool X11Atom::operator==(const X11Atom& other) const {
    return m_value == other.m_value;
}

bool X11Atom::operator==(const Atom& other) const {
    return m_value == other;
}

bool X11Atom::operator==(const std::string_view& other) const {
    return name() == other;
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
        char xmessageBuffer[1024] = {0};
        XGetErrorText(display(), event->error_code, xmessageBuffer, 1024);

        message << xmessageBuffer;
    }

    auto exception = X11Exception(message.str());
    exception.m_errorCode = event->error_code;

    debugStream << "Error during X11 call for display " << errorDisplay << ": " << message.str() << std::endl;
    m_pendingXCallException.emplace(exception);
    return 0;
}

template <typename F, typename... Args>
inline auto X11Connection::doXCall(std::string_view callName, F callLambda, Args... args) {
    throwIfDestroyed();

    if (m_currentXCall.has_value()) {
        throw X11Exception("Tried to call ", callName, " while a call to ", m_currentXCall.value(), " was already in progress");
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

    XSetWindowAttributes attributes {.event_mask = PropertyChangeMask};

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
    return {*this, handle};
}

X11Connection::X11Connection() {
    debugStream << "Opening X11 connection" << std::endl;

    XSetErrorHandler(&globalErrorHandler);

    if (instance != nullptr && instance != this) {
        throw X11Exception("Only one X11 connection can be open at a time");
    }

    m_display = XOpenDisplay(nullptr);
    if (m_display == nullptr) {
        throw X11Exception("XOpenDisplay: failed to open display ", XDisplayName(nullptr));
    }

    // XSynchronize(m_display, True);

    instance = this;
}

X11Connection::~X11Connection() {
    debugStream << "Closing X11 connection" << std::endl;

    X_CALL(XCloseDisplay, m_display);
    m_display = nullptr;
    instance = nullptr;
}

const X11Atom& X11Connection::addAtomToCache(X11Atom&& atom) {
    auto ptr = std::make_shared<X11Atom>(std::move(atom));
    m_atoms_by_name.insert({ptr->name(), ptr});
    m_atoms_by_value.insert({ptr->value(), ptr});
    return *ptr;
}

const X11Atom& X11Connection::atom(std::string_view name) {
    throwIfDestroyed();

    if (m_atoms_by_name.contains(name)) {
        return *m_atoms_by_name.at(name);
    }

    std::string nameCopy {name};
    const auto value = X_CALL(XInternAtom, display(), nameCopy.c_str(), false);
    if (value == None) {
        throw X11Exception("Unable to intern value");
    }

    return addAtomToCache({value, std::move(nameCopy)});
}

const X11Atom& X11Connection::atom(Atom value) {
    throwIfDestroyed();

    if (m_atoms_by_value.contains(value)) {
        return *m_atoms_by_value.at(value);
    }

    auto rawName = X_CALL(XGetAtomName, display(), value);
    if (rawName == nullptr) {
        throw X11Exception("Unable to get atom rawName");
    }
    auto name = capture(rawName);

    return addAtomToCache({value, name.get()});
}

void X11Connection::throwIfDestroyed() const {
    if (m_display == nullptr) {
        throw X11Exception("Tried to use a connection after it was destroyed");
    }
}

XEvent X11Connection::nextEvent() {
    XEvent result;
    X_CALL(XNextEvent, display(), &result);
    return result;
}

std::optional<XEvent> X11Connection::checkMaskEvent(int eventMask) {
    XEvent result;
    if (X_CALL(XCheckMaskEvent, display(), eventMask, &result)) {
        return result;
    }

    return {};
}

Window X11Connection::getSelectionOwner(const X11Atom& selection) {
    throwIfDestroyed();
    return X_CALL(XGetSelectionOwner, display(), selection.value());
}

void X11Connection::sendEvent(Window window, bool propagate, long eventMask, XEvent& event) {
    auto status = X_CALL(XSendEvent, display(), window, propagate, eventMask, &event);
    if (status == 0) {
        throw X11Exception("XSendEvent failed");
    }
}

bool X11Connection::isClipboardOwned() {
    throwIfDestroyed();
    return getSelectionOwner(atom(atomClipboard)) != None;
}

std::shared_ptr<X11Window> X11Connection::externalWindow(Window window) {
    if (m_externalWindows.contains(window)) {
        auto cached = m_externalWindows[window];
        if (!cached.expired()) {
            return cached.lock();
        }

        m_externalWindows.erase(window);
    }

    auto result = std::make_shared<X11Window>(*this, window, false);
    m_externalWindows[window] = result;

    return result;
}

template <>
constexpr X11PropertyFormat X11PropertyFormat::fromSize<sizeof(X11PropertyFormat::format8_t)>() {
    return X11PropertyFormat::Format8;
}

template <>
constexpr X11PropertyFormat X11PropertyFormat::fromSize<sizeof(X11PropertyFormat::format16_t)>() {
    return X11PropertyFormat::Format16;
}

template <>
constexpr X11PropertyFormat X11PropertyFormat::fromSize<sizeof(X11PropertyFormat::format32_t)>() {
    return X11PropertyFormat::Format32;
}

template <std::size_t bad_t>
constexpr X11PropertyFormat X11PropertyFormat::fromSize() {
    static_assert(AssertFalse<bad_t>::value, "Invalid property format size");
    return X11PropertyFormat::Format8; // Just here to make the compiler happy
}

template <ranges::contiguous_range range_t, typename char_t>
X11Property::X11Property(const X11Atom& name, const X11Atom& type, range_t data, bool owned) : X11Property(name, type, X11PropertyFormat::fromSize<sizeof(char_t)>(), data, owned) {}

template <ranges::contiguous_range range_t, typename char_t>
X11Property::X11Property(const X11Atom& name, const X11Atom& type, const X11PropertyFormat& format, range_t data, bool owned)
        : m_name(name)
        , m_type(type)
        , m_format(format)
        , m_size8(data.size() * sizeof(char_t)) {
    if (owned) {
        auto data8 = std::make_unique<X11PropertyFormat::format8_t[]>(m_size8);
        std::memcpy(data8.get(), &data[0], m_size8);
        m_data8.emplace<std::unique_ptr<X11PropertyFormat::format8_t[]>>(std::move(data8));
    } else {
        m_data8.emplace<const X11PropertyFormat::format8_t*>(reinterpret_cast<const X11PropertyFormat::format8_t*>(&data[0]));
    }
}

X11PropertyIterator X11Property::begin() const {
    return {*this, 0};
}

X11PropertyIterator X11Property::end() const {
    return {*this, size()};
}

X11Property X11Property::range(std::size_t start, std::size_t end) {
    start = std::min(start, size());
    end = std::clamp(end, start, size());

    auto begin = data8() + (start * format().size());
    auto count = (end - start) * format().size();
    return {name(), type(), views::counted(begin, count), false};
}

std::partial_ordering X11PropertyIterator::operator<=>(const X11PropertyIterator& other) const {
    if (std::addressof(other.m_property) != std::addressof(m_property)) {
        return std::partial_ordering::unordered;
    }

    return m_offset <=> other.m_offset;
}

X11PropertyFormat::format32_t X11PropertyIterator::operator*() const {
    auto pointer8 = m_property.data8() + (m_property.format().size() * m_offset);

    if (m_property.format() == X11PropertyFormat::Format8) {
        return *pointer8;
    }

    if (m_property.format() == X11PropertyFormat::Format16) {
        return *reinterpret_cast<const X11PropertyFormat::format16_t*>(pointer8);
    }

    if (m_property.format() == X11PropertyFormat::Format32) {
        return *reinterpret_cast<const X11PropertyFormat::format32_t*>(pointer8);
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

X11Window::X11Window(X11Connection& connection, Window window) : X11Window(connection, window, true) {}

X11Window::X11Window(X11Connection& connection, Window window, bool owned) : m_connection(connection), m_window(window), m_owned(owned) {

    if (m_window == None) {
        throw X11Exception("Invalid Window");
    }
}

X11Window::~X11Window() noexcept(false) {
    try {
        clearEventMask();
    } catch (const X11Exception& e) {
        if (e.errorCode() != BadWindow) throw e;
    } // Some platforms throw errors when clearing the event mask here

    debugStream << "Destroying window " << m_window << std::endl;

    if (m_owned) {
        X_CALL(XDestroyWindow, display(), m_window);
        m_window = None;
    }
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

void X11Window::changeProperty(X11PropertyMode mode, const X11Property& value) {
    throwIfDestroyed();

    X_CALL(XChangeProperty, display(), window(), value.name().value(), value.type().value(), value.format().value(), static_cast<int>(mode), value.data8(), value.size());
}

void X11Window::deleteProperty(const X11Atom& property) {
    throwIfDestroyed();
    X_CALL(XDeleteProperty, display(), window(), property.value());
}

template <std::predicate<const XEvent&> predicate_t>
XEvent X11Window::waitForEvent(int eventType, predicate_t predicate) {
    throwIfDestroyed();

    debugStream << "Waiting for event " << eventType << std::endl;
    return pollUntilReturn([this, eventType, &predicate]() -> std::optional<XEvent> {
        auto event = checkTypedWindowEvent(eventType);
        if (event.has_value() && !predicate(event.value())) {
            return {};
        }

        return event;
    });
}

Time X11Window::queryCurrentTime() {
    throwIfDestroyed();

    auto&& name = atom("GETCURRENTTIME");
    X11Property value {name, atom("text/plain"), u8"getcurrenttime"sv, false};

    deleteProperty(name);
    changeProperty(X11PropertyMode::Replace, value);

    const auto event = waitForEvent(PropertyNotify, [&name](const XEvent& event) { return event.xproperty.atom == name.value() && event.xproperty.state == PropertyNewValue; });

    deleteProperty(name);
    return event.xproperty.time;
}

X11Property X11Window::convertSelection(const X11Atom& selection, const X11Atom& target) {
    throwIfDestroyed();

    auto&& property = atom("convertSelectionProperty");
    const auto requestor = window();

    deleteProperty(property);
    X_CALL(XConvertSelection, display(), selection.value(), target.value(), property.value(), requestor, queryCurrentTime());

    const auto result = waitForEvent(SelectionNotify, [requestor, &selection, &target](const XEvent& event) {
        auto& xselection = event.xselection;
        return xselection.requestor == requestor && xselection.selection == selection.value() && xselection.target == target.value();
    });

    if (result.xselection.property == None) {
        throw X11Exception("Selection owner refused selection request");
    }

    return getProperty(property, true);
}

X11Property X11Window::getProperty(const X11Atom& name, bool delet) {
    throwIfDestroyed();

    Atom actualTypeReturn = None;
    int actualFormatReturn = 0;
    unsigned long nitemsReturn = 0;
    unsigned long bytesAfterReturn = 0;
    unsigned char* propReturn = nullptr;

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
           /*prop_return*/ &propReturn);

    // Capture before doing any checks to ensure data will be freed no matter what happens
    auto x11Data = capture(propReturn);

    if (bytesAfterReturn > 0) {
        throw X11Exception("XGetWindowProperty read ", nitemsReturn, " items but left ", bytesAfterReturn, " bytes behind");
    }

    auto&& type = atom(actualTypeReturn);
    const auto format = X11PropertyFormat::fromValue(actualFormatReturn);
    const auto size = nitemsReturn * format.size();

    return X11Property {name, type, format, views::counted(x11Data.get(), size), true};
}

X11Property X11Window::convertClipboard(const X11Atom& target) {
    throwIfDestroyed();
    return convertSelection(atom(atomClipboard), target);
}

std::vector<std::reference_wrapper<const X11Atom>> X11Window::queryClipboardTargets() {
    throwIfDestroyed();

    auto property = convertClipboard(atom(atomTargets));

    std::vector<std::reference_wrapper<const X11Atom>> result {};
    result.reserve(property.size());

    for (auto&& atomValue : property) {
        result.emplace_back(atom(atomValue));
    }

    return result;
}

std::vector<char> X11Window::getClipboardData(const X11Atom& target) {
    throwIfDestroyed();

    std::vector<char> result;
    auto addToResult = [&result](const X11Property& x) {
        for (auto&& c : x) {
            result.push_back(c);
        }
    };

    auto firstResult = convertClipboard(target);
    if (firstResult.type() != atom(atomIncr)) {
        debugStream << "Got a regular non-INCR result" << std::endl;
        addToResult(firstResult);
        return result;
    }

    debugStream << "Got an INCR result" << std::endl;

    while (true) {
        waitForEvent(PropertyNotify, [&](const XEvent& event) { return event.xproperty.atom == firstResult.name().value() && event.xproperty.state == PropertyNewValue; });
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

void X11Window::setSelectionOwner(const X11Atom& selection, Time time) {
    throwIfDestroyed();

    X_CALL(XSetSelectionOwner, display(), selection.value(), window(), time);

    if (connection().getSelectionOwner(selection) != window()) {
        throw X11Exception("XSetSelectionOwner merely appeared to succeed, probably timing issues");
    }
}

void X11Window::changeWindowAttributes(unsigned long valuemask, XSetWindowAttributes* attributes) {
    throwIfDestroyed();

    debugStream << "Setting attributes for window " << window() << std::endl;

    X_CALL(XChangeWindowAttributes, display(), window(), valuemask, attributes);
}

void X11Window::setEventMask(long eventMask) {
    throwIfDestroyed();

    XSetWindowAttributes attributes = {.event_mask = eventMask};

    changeWindowAttributes(CWEventMask, &attributes);
}

XWindowAttributes X11Window::getWindowAttributes() {
    throwIfDestroyed();

    XWindowAttributes attributes;
    auto status = X_CALL(XGetWindowAttributes, display(), window(), &attributes);
    if (status == 0) {
        throw X11Exception("XGetWindowAttributes: failed to get window attributes");
    }

    return attributes;
}

void X11Window::addToEventMask(long eventMask) {
    throwIfDestroyed();

    auto attributes = getWindowAttributes();
    auto newMask = attributes.your_event_mask | eventMask;
    setEventMask(newMask);
}

void X11Window::clearEventMask() {
    throwIfDestroyed();
    setEventMask(NoEventMask);
}

void X11Window::addPropertyChangeToEventMask() {
    throwIfDestroyed();
    addToEventMask(PropertyChangeMask);
}

void X11Window::sendEvent(bool propagate, long eventMask, XEvent& event) {
    throwIfDestroyed();
    connection().sendEvent(window(), propagate, eventMask, event);
}

bool X11Window::operator==(const X11Window& other) const {
    return m_window == other.m_window;
}

bool X11Window::operator==(const Window& other) const {
    return m_window == other;
}

X11SelectionRequest::X11SelectionRequest(XSelectionRequestEvent event, std::shared_ptr<X11Window> window, const X11Atom& target, const X11Atom& property, bool multiple)
        : m_event(event)
        , m_window(std::move(window))
        , m_target(target)
        , m_property(property)
        , m_multiple(multiple) {}

X11SelectionRequest::X11SelectionRequest(X11Connection& connection, XSelectionRequestEvent event)
        : m_event(event)
        , m_window(connection.externalWindow(event.requestor))
        , m_target(connection.atom(event.target))
        , m_property(connection.atom(event.property == None ? event.target : event.property))
        , m_multiple(false) {}

X11SelectionRequest X11SelectionRequest::forMultiple(const X11Atom& target, const X11Atom& property) const {
    return {event(), windowPtr(), target, property, true};
}

X11IncrTransfer::X11IncrTransfer(std::shared_ptr<X11Window> window, X11Property&& property) : m_window(std::move(window)), m_property(std::move(property)), m_offset(0) {}

void X11IncrTransfer::handle(const XEvent& event) {
    if (m_done) {
        return;
    }

    auto isIncr = event.type == PropertyNotify && event.xproperty.window == *m_window && event.xproperty.atom == m_property.name() && event.xproperty.state == PropertyDelete;

    if (!isIncr) {
        return;
    }

    if (m_sentTrailer) {
        debugStream << "INCR:  Final zero-byte property deleted, transfer is over" << std::endl;
        m_done = true;
        return;
    }

    auto partialProp = m_property.range(m_offset, m_offset + chunkSize());
    m_offset += partialProp.size();

    debugStream << "INCR: Sending " << partialProp.size8() << " bytes" << std::endl;
    m_window->changeProperty(X11PropertyMode::Replace, partialProp);

    if (partialProp.size8() == 0) {
        m_sentTrailer = true;
    }
}

X11SelectionDaemon::X11SelectionDaemon(X11Connection& connection, const X11Atom& selection, const ClipboardContent& content)
        : m_connection(connection)
        , m_selection(selection)
        , m_content(content)
        , m_window(connection.createWindow())
        , m_isSelectionOwner(true) {

    debugStream << "Setting the selection owner to ourselves" << std::endl;
    m_selectionAcquiredTime = window().queryCurrentTime();
    window().setSelectionOwner(selection, m_selectionAcquiredTime);
}

XEvent X11SelectionDaemon::nextEvent() {
    if (isSelectionOwner()) {
        return connection().nextEvent();
    }
    // If we don't own the selection anymore, we're only alive to finish serving requests made before
    // we lost the selection ownership. To prevent the daemon from staying up forever, we switch to
    // polling to ensure we'll fail if all ongoing requests are stalled.
    return pollUntilReturn([this]() { return connection().checkMaskEvent(std::numeric_limits<int>::max()); });
}

XEvent X11SelectionDaemon::makeSelectionNotify(const XSelectionRequestEvent& event) {
    return XEvent {
            .xselection = {
                    .type = SelectionNotify,
                    .display = event.display,
                    .requestor = event.requestor,
                    .selection = event.selection,
                    .target = event.target,
                    .property = event.property,
                    .time = event.time,
            }};
}

void X11SelectionDaemon::refuseSelectionRequest(const XSelectionRequestEvent& event) const {
    auto refusal = makeSelectionNotify(event);
    refusal.xselection.property = None;

    connection().sendEvent(event.requestor, false, NoEventMask, refusal);
}

bool X11SelectionDaemon::refuseSelectionRequest(const X11SelectionRequest& request) const {
    auto refusal = makeSelectionNotify(request.event());
    refusal.xselection.property = None;
    request.window().sendEvent(false, NoEventMask, refusal);
    return false;
}

template <ranges::contiguous_range range_t>
bool X11SelectionDaemon::replySelectionRequest(const X11SelectionRequest& request, const X11Atom& type, range_t data) {
    X11Property property {request.property(), type, data, true};
    debugStream << "Replying with " << property.size8() << " bytes of data"
                << " at format " << property.format().value() << " and type " << property.type().name() << std::endl;

    if (data.size() > connection().maxDataSizeForIncr()) {
        debugStream << "Data too big, using INCR mechanism" << std::endl;
        X11Property incrProperty {property.name(), atom(atomIncr), views::single(property.size8()), true};
        request.window().addPropertyChangeToEventMask();
        request.window().changeProperty(X11PropertyMode::Replace, incrProperty);
        m_transfers.emplace_back(std::make_unique<X11IncrTransfer>(request.windowPtr(), std::move(property)));

    } else {
        request.window().changeProperty(X11PropertyMode::Replace, property);
    }

    if (!request.isMultiple()) {
        auto selectionNotify = makeSelectionNotify(request.event());
        request.window().sendEvent(false, NoEventMask, selectionNotify);
    }

    return true;
}

void X11SelectionDaemon::handle(const XEvent& event) {
    if (event.type == SelectionClear) {
        handleSelectionClear(event.xselectionclear);

    } else if (event.type == SelectionRequest) {
        handleSelectionRequest(event.xselectionrequest);
    }
}

void X11SelectionDaemon::handleSelectionClear(const XSelectionClearEvent& event) {
    if (event.selection == selection()) {
        debugStream << "Selection cleared, we are no longer the owners of the selection" << std::endl;
        m_isSelectionOwner = false;
    }
}

void X11SelectionDaemon::handleSelectionRequest(const XSelectionRequestEvent& event) {
    if (!isSelectionOwner()) {
        debugStream << "Selection request received after we lost selection ownership, refusing" << std::endl;
        return refuseSelectionRequest(event);
    }

    if (event.owner != window()) {
        debugStream << "Selection request has incorrect owner window, refusing" << std::endl;
        return refuseSelectionRequest(event);
    }

    if (event.selection != selection()) {
        debugStream << "Selection request has incorrect selection " << atom(event.selection).name() << ", refusing" << std::endl;
        return refuseSelectionRequest(event);
    }

    if (event.time != CurrentTime && event.time < m_selectionAcquiredTime) {
        debugStream << "Selection request time " << event.time << " is from before we acquired selection ownership at " << m_selectionAcquiredTime << ", refusing" << std::endl;
        return refuseSelectionRequest(event);
    }

    if (event.requestor == None) {
        debugStream << "Selection request has no requestor, refusing" << std::endl;
        return refuseSelectionRequest(event);
    }

    handleSelectionRequest({connection(), event});
}

bool X11SelectionDaemon::handleSelectionRequest(const X11SelectionRequest& request) {
    debugStream << "Got a selection request from " << request.window().window() << " for target " << request.target().name() << " on property " << request.property().name() << std::endl;

    if (request.target() == atomMultiple) {
        return handleMultipleSelectionRequest(request);
    }

    if (request.target() == atomTargets) {
        return handleTargetsSelectionRequest(request);
    }

    if (request.target() == atomTimestamp) {
        return handleTimestampSelectionRequest(request);
    }

    return handleRegularSelectionRequest(request);
}

bool X11SelectionDaemon::handleMultipleSelectionRequest(const X11SelectionRequest& request) {
    std::optional<X11Property> pairs;
    try {
        pairs.emplace(request.window().getProperty(request.property()));
    } catch (const X11Exception& e) {
        debugStream << "Error trying to get MULTIPLE atom pair: " << e.what() << ", refusing" << std::endl;
        return refuseSelectionRequest(request);
    }

    if (pairs->type() != atomAtomPair) {
        debugStream << "MULTIPLE property parameter isn't an atom pair, refusing" << std::endl;
        return refuseSelectionRequest(request);
    }

    std::vector<X11PropertyFormat::format32_t> result;
    std::optional<X11PropertyFormat::format32_t> target;
    for (auto&& value : *pairs) {
        if (!target) {
            target = value;
            result.push_back(value);
            continue;
        }

        auto newRequest = request.forMultiple(atom(*target), atom(value));
        if (handleSelectionRequest(newRequest)) {
            result.push_back(value);
        } else {
            result.push_back(None);
        }
    }

    return replySelectionRequest(request, atom(atomAtomPair), result);
}

bool X11SelectionDaemon::handleTargetsSelectionRequest(const X11SelectionRequest& request) {
    std::vector<X11PropertyFormat::format32_t> data {
            atom(atomTargets).value(),
            atom(atomMultiple).value(),
            atom(atomTimestamp).value(),
    };
    MimeType::forEachSupporting(content(), [&](const MimeType& type) { data.push_back(atom(type.name()).value()); });

    for (auto&& value : data) {
        debugStream << "Advertising target: " << atom(value).name() << std::endl;
    }

    return replySelectionRequest(request, atom(atomAtom), data);
}

bool X11SelectionDaemon::handleTimestampSelectionRequest(const X11SelectionRequest& event) {
    debugStream << "Got a TIMESTAMP request" << std::endl;
    debugStream << "Replying with: " << m_selectionAcquiredTime << std::endl;

    return replySelectionRequest(event, atom(atomInteger), ranges::single_view(m_selectionAcquiredTime));
}

bool X11SelectionDaemon::handleRegularSelectionRequest(const X11SelectionRequest& request) {

    auto mime = request.target().name();
    std::ostringstream stream;
    if (MimeType::encode(content(), mime, stream)) {
        return replySelectionRequest(request, atom(mime), stream.str());
    }

    debugStream << "Unable to encode clipboard content, refusing" << std::endl;
    return refuseSelectionRequest(request);
}

void X11SelectionDaemon::run() {
    debugStream << "Starting persistent paste daemon" << std::endl;

    kill(getppid(), SIGUSR1);

    while (true) {
        auto event = nextEvent();
        handle(event);
        for (auto&& transfer : m_transfers)
            transfer->handle(event);

        std::erase_if(m_transfers, [](auto&& transfer) { return transfer->isDone(); });

        if (!m_transfers.empty()) {
            debugStream << m_transfers.size() << " transfers are in progress" << std::endl;
        }

        if (!isSelectionOwner() && m_transfers.empty()) {
            debugStream << "Ownership lost and transfers are done, exiting" << std::endl;
            break;
        }
    }
}

static ClipboardContent getX11ClipboardInternal(const std::string& requested_mime) {
    X11Connection conn;
    if (!conn.isClipboardOwned()) {
        debugStream << "No selection owner, aborting" << std::endl;
        return {};
    }

    auto window = conn.createWindow();

    auto offeredTargets = window.queryClipboardTargets();
    auto offeredTypes = views::transform(offeredTargets, [](auto&& x) { return x.get().name(); });

    std::vector<char> data;
    std::istringstream stream;
    auto request = [&](const MimeType& type) -> std::istream& {
        data = window.getClipboardData(conn.atom(type.name()));
        stream = std::istringstream {std::string {data.data(), data.size()}};
        return stream;
    };

    auto content = MimeType::decode(offeredTypes, request, requested_mime);

    std::vector<std::string> mimes(offeredTypes.begin(), offeredTypes.end());

    content.makeTypesAvailable(mimes);

    return content;
}

static void startPasteDaemon(const ClipboardContent& clipboard) {
    X11Connection conn;
    X11SelectionDaemon daemon {conn, conn.atom(atomClipboard), clipboard};
    XSynchronize(conn.display(), True);
    daemon.run();
}

static bool setX11ClipboardInternal(const WriteGuiContext& context) {
    context.forker.fork([&]() { startPasteDaemon(context.clipboard); });
    return waitForSuccessSignal();
}

extern "C" {
extern void* getX11Clipboard(void* ptr) {
    try {
        std::string requested_mime(*reinterpret_cast<std::string*>(ptr));
        auto clipboard = std::make_unique<ClipboardContent>(getX11ClipboardInternal(requested_mime));
        return clipboard.release();
    } catch (const std::exception& e) {
        debugStream << "Error getting clipboard data: " << e.what() << std::endl;
        return nullptr;
    }
}

extern bool setX11Clipboard(void* ptr) {
    try {
        const WriteGuiContext& context = *reinterpret_cast<WriteGuiContext*>(ptr);
        return setX11ClipboardInternal(context);
    } catch (const std::exception& e) {
        debugStream << "Error setting clipboard data: " << e.what() << std::endl;
        return false;
    }
}
}
