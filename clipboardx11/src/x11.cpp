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
#include <utility>
#include <variant>
#include <vector>
#include <limits>
#include <set>

#include <X11/Xlib.h>
#include <clipboard/gui.hpp>
#include <clipboard/logging.hpp>

namespace chrono = std::chrono;
namespace ranges = std::ranges;
namespace views = std::views;

using namespace std::literals;

constexpr auto maxEventPollTime = 5s;
constexpr auto startEventPollBackoff = 1ms;
constexpr auto eventPollBackoffMultiplier = 2;
constexpr auto maxEventPollBackoffTime = 500ms;

constexpr auto atomClipboard = "CLIPBOARD";
constexpr auto atomTargets = "TARGETS";
constexpr auto atomMultiple = "MULTIPLE";
constexpr auto atomTimestamp = "TIMESTAMP";
constexpr auto atomIncr = "INCR";
constexpr auto atomAtom = "ATOM";
constexpr auto atomAtomPair = "ATOM_PAIR";
constexpr auto atomInteger = "INTEGER";

// Forward declarations
class X11ClipboardType;
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

enum class X11ClipboardTypeOption;
enum class X11PropertyMode;

#define X_CALL(name, ...) doXCall(#name, &name, __VA_ARGS__)

template<typename T>
using X11Pointer = std::unique_ptr<T, decltype(&XFree)>;

template<typename T>
X11Pointer<T> capture(T* ptr) {
    return { ptr, &XFree };
}

enum class X11ClipboardTypeOption {
    NoOption = 0,
    ChooseBestType = 1 << 1,
    IncludeAction = 1 << 2,
    EncodePaths = 1 << 3,
};
static X11ClipboardTypeOption operator|(X11ClipboardTypeOption const& a, X11ClipboardTypeOption const& b) {
    return static_cast<X11ClipboardTypeOption>(static_cast<int>(a) | static_cast<int>(b));
}

class X11ClipboardType {
private:
    unsigned int const m_priority;
    char const* const m_name;
    ClipboardContentType const m_type;
    X11ClipboardTypeOption m_options;

public:
    X11ClipboardType(
        unsigned int priority,
        char const* const name,
        ClipboardContentType type,
        X11ClipboardTypeOption options)
        : m_priority(priority)
        , m_name(name)
        , m_type(type)
        , m_options(options) { }

    [[nodiscard]] inline unsigned int priority() const { return m_priority; }
    [[nodiscard]] inline char const* const name() const { return m_name; }
    [[nodiscard]] inline ClipboardContentType type() const { return m_type; }
    [[nodiscard]] inline bool isChooseBestType() const { return (static_cast<int>(m_options) & static_cast<int>(X11ClipboardTypeOption::ChooseBestType)) != 0; }
    [[nodiscard]] inline bool isIncludeAction() const { return (static_cast<int>(m_options) & static_cast<int>(X11ClipboardTypeOption::IncludeAction)) != 0; }
    [[nodiscard]] inline bool isEncodePaths() const { return (static_cast<int>(m_options) & static_cast<int>(X11ClipboardTypeOption::EncodePaths)) != 0; }

    bool supports(ClipboardContent const&) const;

private:
    inline static std::map<std::string_view, X11ClipboardType> s_typesByName {};
    inline static std::vector<std::tuple<char const* const, ClipboardContentType, X11ClipboardTypeOption>> const s_knownTypes {
        { "x-special/gnome-copied-files", ClipboardContentType::Paths, X11ClipboardTypeOption::IncludeAction | X11ClipboardTypeOption::EncodePaths },
        { "text/uri-list", ClipboardContentType::Paths, X11ClipboardTypeOption::NoOption | X11ClipboardTypeOption::EncodePaths },
        { "text/plain;charset=utf-8", ClipboardContentType::Text, X11ClipboardTypeOption::NoOption },
        { "UTF8_STRING", ClipboardContentType::Text, X11ClipboardTypeOption::NoOption },
        { "text/plain", ClipboardContentType::Text, X11ClipboardTypeOption::NoOption },
        { "STRING", ClipboardContentType::Text, X11ClipboardTypeOption::NoOption },
        { "TEXT", ClipboardContentType::Text, X11ClipboardTypeOption::ChooseBestType },
    };

    static void tryPopulateTypesByName();
public:
    static auto all() { return views::values(s_typesByName); }
    static std::optional<X11ClipboardType> find(X11Atom const&);
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
    bool operator==(Atom const& other) const;
    bool operator==(std::string_view const& other) const;
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
    std::map<Window, std::weak_ptr<X11Window>> m_externalWindows;

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
    [[nodiscard]] inline std::size_t maxRequestSize() const { return XMaxRequestSize(display()); }
    [[nodiscard]] inline std::size_t maxDataSizeForIncr() const { return maxRequestSize() / 2; }

    X11Atom const& atom(char const*);
    X11Atom const& atom(Atom);

    [[nodiscard]] XEvent nextEvent();
    [[nodiscard]] std::optional<XEvent> checkMaskEvent(int eventMask);
    Window getSelectionOwner(X11Atom const&);
    void sendEvent(Window, bool propagate, long eventMask, XEvent& event);
    bool isClipboardOwned();

    X11Window createWindow();
    std::shared_ptr<X11Window> externalWindow(Window);
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

    static X11PropertyFormat fromSize(std::size_t size) {
        if (size == sizeof(std::uint8_t)) {
            return Format8;
        }
        if (size == sizeof(std::uint16_t)) {
            return Format16;
        }
        if (size == sizeof(std::uint64_t)) {
            return Format32;
        }

        throw X11Exception("Invalid format size");
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
    X11PropertyFormat m_format;

    std::variant<std::uint8_t const*, std::unique_ptr<std::uint8_t[]>> m_data8;
    std::size_t m_size8;

public:
    template<ranges::contiguous_range range_t, typename char_t = ranges::range_value_t<range_t>>
    X11Property(X11Atom const& name, X11Atom const& type, range_t data, bool owned);

    template<ranges::contiguous_range range_t, typename char_t = ranges::range_value_t<range_t>>
    X11Property(X11Atom const& name, X11Atom const& type, X11PropertyFormat const&, range_t data, bool owned);

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

    [[nodiscard]] X11Property range(std::size_t start, std::size_t end);
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
    ~X11Window();

    [[nodiscard]] inline X11Connection& connection() const { return m_connection; }
    [[nodiscard]] inline Display* display() const { return m_connection.display(); }
    [[nodiscard]] inline Window window() const { return m_window; }

    [[nodiscard]] inline X11Atom const& atom(char const* name) const { return m_connection.atom(name); }
    [[nodiscard]] inline X11Atom const& atom(Atom value) const { return m_connection.atom(value); }

    [[nodiscard]] Time queryCurrentTime();
    [[nodiscard]] std::optional<XEvent> checkTypedWindowEvent(int eventType);
    [[nodiscard]] std::optional<XEvent> checkMaskEvent(int eventMask);
    void changeProperty(X11PropertyMode, X11Property const&);
    void deleteProperty(X11Atom const&);
    [[nodiscard]] X11Property getProperty(X11Atom const&, bool delet = false);
    [[nodiscard]] X11Property convertSelection(X11Atom const& selection, X11Atom const& target);
    void setSelectionOwner(X11Atom const& selection, Time time);
    XWindowAttributes getWindowAttributes();
    void changeWindowAttributes(unsigned long valuemask, XSetWindowAttributes* attributes);
    void sendEvent(bool propagate, long eventMask, XEvent& event);

    [[nodiscard]] std::vector<std::reference_wrapper<X11Atom const>> queryClipboardTargets();
    [[nodiscard]] X11Property convertClipboard(X11Atom const& target);
    [[nodiscard]] std::vector<char> getClipboardData(X11Atom const& target);
    void setEventMask(long);
    void addToEventMask(long);
    void addPropertyChangeToEventMask();
    void clearEventMask();

    template<std::predicate<XEvent const&> predicate_t>
    XEvent waitForEvent(int eventType, predicate_t predicate);

    template<typename F, typename... Args>
    inline auto doXCall(std::string_view callName, F callLambda, Args... args) {
        return connection().doXCall(callName, callLambda, args...);
    }

    bool operator==(X11Window const&) const;
    bool operator==(Window const&) const;
};

class X11SelectionRequest {
private:
    XSelectionRequestEvent m_event;
    std::shared_ptr<X11Window> m_window;
    X11Atom const& m_target;
    X11Atom const& m_property;
    bool m_multiple;

    X11SelectionRequest(
        XSelectionRequestEvent,
        std::shared_ptr<X11Window>,
        X11Atom const& target,
        X11Atom const& property,
        bool multiple
    );

public:
    X11SelectionRequest(X11Connection&, XSelectionRequestEvent);

    [[nodiscard]] inline XSelectionRequestEvent const& event() const { return m_event; }
    [[nodiscard]] inline X11Connection& connection() const { return m_window->connection(); }
    [[nodiscard]] inline X11Window& window() const { return *m_window; }
    [[nodiscard]] inline std::shared_ptr<X11Window> windowPtr() const { return m_window; }
    [[nodiscard]] inline X11Atom const& target() const { return m_target; }
    [[nodiscard]] inline X11Atom const& property() const { return m_property; }
    [[nodiscard]] inline bool isMultiple() const { return m_multiple; }

    X11SelectionRequest forMultiple(X11Atom const& target, X11Atom const& property) const;
};

class X11SelectionTransfer {
protected:
    bool m_done = false;

public:
    [[nodiscard]] inline bool isDone() const { return m_done; }
    virtual void handle(XEvent const&) = 0;
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
    void handle(XEvent const&) override;
};

class X11SelectionDaemon {
private:
    X11Connection& m_connection;
    X11Atom const& m_selection;
    ClipboardContent const& m_content;

    X11Window m_window;
    Time m_selectionAcquiredTime;
    bool m_isSelectionOwner;

    std::vector<std::unique_ptr<X11SelectionTransfer>> m_transfers;

    XEvent nextEvent();
    static XEvent makeSelectionNotify(XSelectionRequestEvent const&);
    void refuseSelectionRequest(XSelectionRequestEvent const&) const;
    bool refuseSelectionRequest(X11SelectionRequest const&) const;

    template<ranges::contiguous_range range_t>
    bool replySelectionRequest(X11SelectionRequest const&, X11Atom const& type, range_t);

    void handle(XEvent const&);
    void handleSelectionClear(XSelectionClearEvent const&);
    void handleSelectionRequest(XSelectionRequestEvent const&);
    bool handleSelectionRequest(X11SelectionRequest const&);

    bool handleMultipleSelectionRequest(X11SelectionRequest const&);
    bool handleTimestampSelectionRequest(X11SelectionRequest const&);
    bool handleTargetsSelectionRequest(X11SelectionRequest const&);
    bool handleRegularSelectionRequest(X11SelectionRequest const&);
    bool handlePathsSelectionRequest(X11SelectionRequest const&, X11ClipboardType const&);
    bool handleTextSelectionRequest(X11SelectionRequest const&, X11ClipboardType const&);

public:
    explicit X11SelectionDaemon(X11Connection&, X11Atom const& selection, ClipboardContent const&);

    [[nodiscard]] inline X11Connection& connection() const { return m_connection; }
    [[nodiscard]] inline X11Atom const& selection() const { return m_selection; }
    [[nodiscard]] inline X11Window& window() { return m_window; }
    [[nodiscard]] inline ClipboardContent const& content() const { return m_content; }
    [[nodiscard]] inline bool isSelectionOwner() const { return m_isSelectionOwner; }

    [[nodiscard]] inline X11Atom const& atom(char const* name) const { return m_connection.atom(name); }
    [[nodiscard]] inline X11Atom const& atom(Atom value) const { return m_connection.atom(value); }

    void run();
};

template<typename Return, typename Func>
static Return poll(Func func) {
    auto const startTime = chrono::steady_clock::now();
    auto backoffTime = startEventPollBackoff;

    std::optional<Return> result;
    while (!(result = func()).has_value()) {
        debugStream << "No poll data, sleeping" << std::endl;

        auto const time = chrono::steady_clock::now() - startTime;
        if (time >= maxEventPollTime) {
            debugStream << "Timeout during poll" << std::endl;
            throw X11Exception("Timeout during poll");
        }

        std::this_thread::sleep_for(backoffTime);
        backoffTime = eventPollBackoffMultiplier * backoffTime;
        if (backoffTime > maxEventPollBackoffTime) {
            backoffTime = maxEventPollBackoffTime;
        }
    }

    debugStream << "Poll finished successfully, got a result" << std::endl;
    return result.value();
}

static std::string urlEncode(std::string const& value) {
    static std::set<char> const allowedCharacters {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',

        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',

        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '-', '_', '.', '~', '/'
    };

    std::stringstream result;
    for (auto&& c : value) {
        if (allowedCharacters.contains(c)) {
            result << c;
            continue;
        }

        result
                << "%"
                << std::hex
                << std::uppercase
                << std::setw(2)
                << std::setfill('0')
                << static_cast<std::uint64_t>(static_cast<std::uint8_t>(c));
    }

    return result.str();
}

void X11ClipboardType::tryPopulateTypesByName() {
    if (!s_typesByName.empty()) {
        return;
    }

    unsigned int priority = 0;
    for (auto&& [ name, type, options ] : s_knownTypes) {
        s_typesByName.insert({ name, { priority++, name, type, options } });
    }
}

std::optional<X11ClipboardType> X11ClipboardType::find(X11Atom const& atom) {
    tryPopulateTypesByName();

    auto&& it = s_typesByName.find(atom.name());
    if (it == s_typesByName.end()) {
        return {};
    }

    return it->second;
}

std::optional<X11ClipboardType> X11ClipboardType::findBest(std::vector<std::reference_wrapper<const X11Atom>> const& targets) {
    tryPopulateTypesByName();

    std::optional<X11ClipboardType> best {};
    for (auto&& target : targets) {
        debugStream << "Advertised target: " << target.get().name() << std::endl;

        auto found = find(target.get());
        if (!found.has_value()) {
            continue;
        }

        if (best.has_value() && best->priority() <= found->priority()) {
            continue;
        }

        best.emplace(*found);
    }

    return best;
}

bool X11ClipboardType::supports(ClipboardContent const& content) const {
    if (type() == content.type()) {
        return true;
    }

    if (type() == ClipboardContentType::Text && content.type() == ClipboardContentType::Paths) {
        return true;
    }

    return false;
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

bool X11Atom::operator==(Atom const& other) const {
    return m_value == other;
}

bool X11Atom::operator==(std::string_view const& other) const {
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
    auto status = X_CALL(XSendEvent,
        display(),
        window,
        propagate,
        eventMask,
        &event
    );
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

template<ranges::contiguous_range range_t, typename char_t>
X11Property::X11Property(
    X11Atom const& name,
    X11Atom const& type,
    range_t data,
    bool owned
) : X11Property(
    name,
    type,
    X11PropertyFormat::fromSize(sizeof(char_t)),
    data,
    owned
) { }

template<ranges::contiguous_range range_t, typename char_t>
X11Property::X11Property(
    X11Atom const& name,
    X11Atom const& type,
    X11PropertyFormat const& format,
    range_t data,
    bool owned
) : m_name(name)
  , m_type(type)
  , m_format(format)
  , m_size8(data.size() * sizeof(char_t)) {
    if (owned) {
        auto data8 = std::make_unique<std::uint8_t[]>(m_size8);
        std::memcpy(data8.get(), &data[0], m_size8);
        m_data8.emplace<std::unique_ptr<std::uint8_t[]>>(std::move(data8));
    } else {
        m_data8.emplace<std::uint8_t const*>(reinterpret_cast<std::uint8_t const*>(&data[0]));
    }
}

X11PropertyIterator X11Property::begin() const {
    return { *this, 0 };
}

X11PropertyIterator X11Property::end() const {
    return { *this, size() };
}

X11Property X11Property::range(std::size_t start, std::size_t end) {
    start = std::min(start, size());
    end = std::clamp(end, start, size());

    auto begin = data8() + (start * format().size());
    auto count = (end - start) * format().size();
    return {
        name(),
        type(),
        views::counted(begin, count),
        false
    };
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
    : X11Window(connection, window, true) { }

X11Window::X11Window(X11Connection& connection, Window window, bool owned)
        : m_connection(connection)
        , m_window(window)
        , m_owned(owned) {

    if (m_window == None) {
        throw X11Exception("Invalid Window");
    }
}

X11Window::~X11Window() {
    clearEventMask();

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

void X11Window::changeProperty(X11PropertyMode mode, X11Property const& value) {
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

template<std::predicate<XEvent const&> predicate_t>
XEvent X11Window::waitForEvent(int eventType, predicate_t predicate) {
    throwIfDestroyed();

    debugStream << "Waiting for event " << eventType << std::endl;
    return poll<XEvent>([this, eventType, &predicate]() -> std::optional<XEvent> {
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
    X11Property value { name, atom("text/plain"), u8"getcurrenttime"sv, false };

    deleteProperty(name);
    changeProperty(X11PropertyMode::Replace, value);

    auto const event = waitForEvent(PropertyNotify, [&name](XEvent const& event) {
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

    auto const result = waitForEvent(SelectionNotify, [requestor, &selection, &target](XEvent const& event) {
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

    return X11Property {
        name,
        type,
        format,
        views::counted(x11Data.get(), size),
        true
    };
}

X11Property X11Window::convertClipboard(const X11Atom &target) {
    throwIfDestroyed();
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

void X11Window::setSelectionOwner(const X11Atom &selection, Time time) {
    throwIfDestroyed();

    X_CALL(XSetSelectionOwner, display(), selection.value(), window(), time);

    if (connection().getSelectionOwner(selection) != window()) {
        throw X11Exception("XSetSelectionOwner merely appeared to succeed, probably timing issues");
    }
}

void X11Window::changeWindowAttributes(unsigned long valuemask, XSetWindowAttributes* attributes) {
    throwIfDestroyed();

    X_CALL(XChangeWindowAttributes,
       display(),
       window(),
       valuemask,
       attributes
   );
}

void X11Window::setEventMask(long eventMask) {
    throwIfDestroyed();

    XSetWindowAttributes attributes = {
            .event_mask = eventMask
    };

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

bool X11Window::operator==(X11Window const& other) const {
    return m_window == other.m_window;
}

bool X11Window::operator==(Window const& other) const {
    return m_window == other;
}

X11SelectionRequest::X11SelectionRequest(
    XSelectionRequestEvent event,
    std::shared_ptr<X11Window> window,
    X11Atom const& target,
    X11Atom const& property,
    bool multiple)
    : m_event(event)
    , m_window(std::move(window))
    , m_target(target)
    , m_property(property)
    , m_multiple(multiple) { }

X11SelectionRequest::X11SelectionRequest(
    X11Connection& connection,
    XSelectionRequestEvent event)
    : m_window(connection.externalWindow(event.requestor))
    , m_property(connection.atom(event.property == None ? event.target : event.property))
    , m_target(connection.atom(event.target))
    , m_event(event)
    , m_multiple(false) {

}

X11SelectionRequest X11SelectionRequest::forMultiple(X11Atom const& target, X11Atom const& property) const {
    return {
        event(),
        windowPtr(),
        target,
        property,
        true
    };
}

X11IncrTransfer::X11IncrTransfer(
    std::shared_ptr<X11Window> window,
    X11Property&& property
) : m_window(std::move(window))
  , m_property(std::move(property))
  , m_offset(0) {

}

void X11IncrTransfer::handle(XEvent const& event) {
    if (m_done) {
        return;
    }

    auto isIncr =
            event.type == PropertyNotify
            && event.xproperty.window == *m_window
            && event.xproperty.atom == m_property.name()
            && event.xproperty.state == PropertyDelete;

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

X11SelectionDaemon::X11SelectionDaemon(
        X11Connection& connection,
        X11Atom const& selection,
        ClipboardContent const& content)
    : m_connection(connection)
    , m_selection(selection)
    , m_window(connection.createWindow())
    , m_content(content)
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

    return poll<XEvent>([this]() {
        return connection().checkMaskEvent(std::numeric_limits<int>::max());
    });
}

XEvent X11SelectionDaemon::makeSelectionNotify(XSelectionRequestEvent const& event) {
    return XEvent {
        .xselection = {
            .type = SelectionNotify,
            .display = event.display,
            .requestor = event.requestor,
            .selection = event.selection,
            .target = event.target,
            .property = event.property,
            .time = event.time,
        }
    };
}

void X11SelectionDaemon::refuseSelectionRequest(XSelectionRequestEvent const& event) const {
    auto refusal = makeSelectionNotify(event);
    refusal.xselection.property = None;

    connection().sendEvent(
        event.requestor,
        false,
        NoEventMask,
        refusal
    );
}

bool X11SelectionDaemon::refuseSelectionRequest(X11SelectionRequest const& request) const {
    auto refusal = makeSelectionNotify(request.event());
    refusal.xselection.property = None;
    request.window().sendEvent(false, NoEventMask, refusal);
    return false;
}

template<ranges::contiguous_range range_t>
bool X11SelectionDaemon::replySelectionRequest(X11SelectionRequest const& request, X11Atom const& type, range_t data) {
    X11Property property {
        request.property(),
        type,
        data,
        true
    };
    debugStream
        << "Replying with " << property.size8() << " bytes of data"
        << " at format " << property.format().value()
        << " and type " << property.type().name()
        << std::endl;

    if (data.size() > connection().maxDataSizeForIncr()) {
        debugStream << "Data too big, using INCR mechanism" << std::endl;
        X11Property incrProperty {
            property.name(),
            atom(atomIncr),
            views::single(property.size8()),
            true
        };
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

void X11SelectionDaemon::handle(XEvent const& event) {
    if (event.type == SelectionClear) {
        handleSelectionClear(event.xselectionclear);

    } else if (event.type == SelectionRequest) {
        handleSelectionRequest(event.xselectionrequest);

    }
}

void X11SelectionDaemon::handleSelectionClear(XSelectionClearEvent const& event) {
    if (event.selection == selection()) {
        debugStream << "Selection cleared, we are no longer the owners of the selection" << std::endl;
        m_isSelectionOwner = false;
    }
}

void X11SelectionDaemon::handleSelectionRequest(XSelectionRequestEvent const& event) {
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
        debugStream << "Selection request time " << event.time << " is from before we acquired selection ownership at " << m_selectionAcquiredTime <<", refusing" << std::endl;
        return refuseSelectionRequest(event);
    }

    if (event.requestor == None) {
        debugStream << "Selection request has no requestor, refusing" << std::endl;
        return refuseSelectionRequest(event);
    }

    handleSelectionRequest({ connection(), event });
}

bool X11SelectionDaemon::handleSelectionRequest(X11SelectionRequest const& request) {
    debugStream
        << "Got a selection request from " << request.window().window()
        << " for target " << request.target().name()
        << " on property " << request.property().name()
        << std::endl;

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

bool X11SelectionDaemon::handleMultipleSelectionRequest(X11SelectionRequest const& request) {
    std::optional<X11Property> pairs;
    try {
        pairs.emplace(request.window().getProperty(request.property()));
    } catch (X11Exception const& e) {
        debugStream << "Error trying to get MULTIPLE atom pair: " << e.what() << ", refusing" << std::endl;
        return refuseSelectionRequest(request);
    }

    if (pairs->type() != atomAtomPair) {
        debugStream << "MULTIPLE property parameter isn't an atom pair, refusing" << std::endl;
        return refuseSelectionRequest(request);
    }

    std::vector<std::uint64_t> result;
    std::optional<std::uint64_t> target;
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

bool X11SelectionDaemon::handleTargetsSelectionRequest(X11SelectionRequest const& request) {
    std::vector<std::uint64_t> data {
        atom(atomTargets).value(),
        atom(atomMultiple).value(),
        atom(atomTimestamp).value(),
    };
    for (auto&& type : X11ClipboardType::all()) {
        if (type.supports(content())) {
            data.push_back(atom(type.name()).value());
        }
    }

    for (auto&& value : data) {
        debugStream << "Advertising target: " << atom(value).name() << std::endl;
    }

    return replySelectionRequest(request, atom(atomAtom), data);
}

bool X11SelectionDaemon::handleTimestampSelectionRequest(X11SelectionRequest const& event) {
    debugStream << "Got a TIMESTAMP request" << std::endl;
    debugStream << "Replying with: " << m_selectionAcquiredTime << std::endl;

    std::vector<std::uint64_t> data { m_selectionAcquiredTime };
    return replySelectionRequest(event, atom(atomInteger), data);
}

bool X11SelectionDaemon::handleRegularSelectionRequest(X11SelectionRequest const& request) {
    auto type = X11ClipboardType::find(request.target());
    if (!type.has_value()) {
        debugStream << "Target not recognized, refusing" << std::endl;
        return refuseSelectionRequest(request);
    }

    if (type->isChooseBestType()) {
        auto all = X11ClipboardType::all();
        auto bestType = std::find_if(all.begin(), all.end(), [&](X11ClipboardType const& value) -> bool {
            return value.supports(content()) && !value.isChooseBestType();
        });

        if (bestType == all.end()) {
            throw X11Exception("Unable to find proper target");
        }

        type.emplace(*bestType);
    }

    if (!type->supports(content())) {
        debugStream << "Target is incompatible with the clipboard content, refusing" << std::endl;
        return refuseSelectionRequest(request);
    }

    if (content().type() == ClipboardContentType::Paths) {
        return handlePathsSelectionRequest(request, *type);
    } else if (content().type() == ClipboardContentType::Text) {
        return handleTextSelectionRequest(request, *type);
    } else {
        debugStream << "Unknown clipboard content, refusing" << std::endl;
        return refuseSelectionRequest(request);
    }
}

bool X11SelectionDaemon::handlePathsSelectionRequest(X11SelectionRequest const& request, X11ClipboardType const& type) {
    auto&& paths = content().paths();

    std::stringstream data;
    if (type.isIncludeAction()) {
        if (paths.action() == ClipboardPathsAction::Cut) {
            data << "cut" << std::endl;
        } else {
            data << "copy" << std::endl;
        }
    }

    bool first = true;
    for (auto&& path : paths.paths()) {
        if (!first) {
            data << std::endl;
        }

        if (type.isEncodePaths()) {
            data << "file://" << urlEncode(path.string());
        } else {
            data << path.string();
        }

        first = false;
    }

    debugStream << "Replying with: " << std::endl << data.str() << std::endl;
    return replySelectionRequest(
            request,
            atom(type.name()),
            data.str()
    );
}

bool X11SelectionDaemon::handleTextSelectionRequest(X11SelectionRequest const& request, X11ClipboardType const& type) {
    return replySelectionRequest(
        request,
        atom(type.name()),
        content().text()
    );
}

void X11SelectionDaemon::run() {
    debugStream << "Starting persistent paste daemon" << std::endl;

    while (true) {
        auto event = nextEvent();
        handle(event);

        for (auto&& transfer : m_transfers) {
            transfer->handle(event);
        }

        std::erase_if(m_transfers, [](auto&& transfer) {
            return transfer->isDone();
        });

        if (!m_transfers.empty()) {
            debugStream << m_transfers.size() << " transfers are in progress" << std::endl;
        }

        if (!isSelectionOwner() && m_transfers.empty()) {
            debugStream << "Ownership lost and transfers are done, exiting" << std::endl;
            break;
        }
    }
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
    debugStream << "Parsing buffer as files, raw data:" << std::endl << dataStr << std::endl;

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

    return { std::move(paths), action };
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

static void startPasteDaemon(ClipboardContent const& clipboard) {
    X11Connection conn;
    X11SelectionDaemon daemon { conn, conn.atom(atomClipboard), clipboard };
    daemon.run();
}

static void setX11ClipboardInternal(ClipboardContent const& clipboard) {
    bool noFork = std::getenv("CLIPBOARD_X11_NO_FORK") != nullptr;
    if (!noFork && fork() != 0) {
        debugStream << "Successfully spawned X11 paste daemon" << std::endl;
        return;
    }

    debugStream << "We are the X11 paste daemon, hijacking operation" << std::endl;
    try {
        startPasteDaemon(clipboard);
    } catch (std::exception const& e) {
        debugStream << "Error during X11 daemon operation: " << e.what() << std::endl;
    }

    // Always exit no matter what happens, to prevent the forked daemon
    // from returning control to the stack frames above and overwriting the
    // non-forked original process' work
    std::quick_exit(EXIT_SUCCESS);
}

extern "C" {
    extern void* getX11Clipboard() {
        try {
            auto clipboard = std::make_unique<ClipboardContent>(getX11ClipboardInternal());
            return clipboard.release();
        } catch (std::exception const& e) {
            debugStream << "Error getting clipboard data: " << e.what() << std::endl;
            return nullptr;
        }
    }

    extern void setX11Clipboard(void* clipboardPtr) {
        try {
            ClipboardContent const& clipboard = *reinterpret_cast<ClipboardContent const*>(clipboardPtr);
            setX11ClipboardInternal(clipboard);
        } catch (std::exception const& e) {
            debugStream << "Error setting clipboard data: " << e.what() << std::endl;
        }
    }
}
