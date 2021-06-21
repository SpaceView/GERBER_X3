#pragma once

#include <QString>
#include <chrono>
#include <map>
#include <string_view>

using nanoseconds = std::nano;
using microseconds = std::micro;
using milliseconds = std::milli;
using seconds = std::ratio<1>;
using minutes = std::ratio<60>;
using hours = std::ratio<3600>;
using std::is_same_v;
template <class T = seconds>
    requires //
    is_same_v<T, nanoseconds> || //
    is_same_v<T, microseconds> || //
    is_same_v<T, milliseconds> || //
    is_same_v<T, seconds> || //
    is_same_v<T, minutes> || //
    is_same_v<T, hours> //
    struct Timer {
#ifdef __gnu_linux__
    const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> t1;
#else
    const std::chrono::time_point<std::chrono::steady_clock> t1;
#endif
    const std::string_view stringView;
    static inline std::map<std::string_view, std::pair<size_t, double>> avgMap;

    constexpr Timer(std::string_view name, T = {})
        : t1 { std::chrono::high_resolution_clock::now() }
        , stringView { name }
    {
    }

    ~Timer()
    {
        using std::chrono::duration;
        using std::chrono::high_resolution_clock;

        duration<double, T> timeout { high_resolution_clock::now() - t1 };

        auto& [ctr, avg] = avgMap[stringView];
        avg += timeout.count();
        ++ctr;

        /**/ if constexpr (std::is_same_v<T, nanoseconds>)
            qDebug("%s\n-> %1.3f (avg %1.3f) nanoseconds", stringView.data(), timeout.count(), avg / ctr);
        else if constexpr (std::is_same_v<T, microseconds>)
            qDebug("%s\n-> %1.3f (avg %1.3f) microseconds", stringView.data(), timeout.count(), avg / ctr);
        else if constexpr (std::is_same_v<T, milliseconds>)
            qDebug("%s\n-> %1.3f (avg %1.3f) milliseconds", stringView.data(), timeout.count(), avg / ctr);
        else if constexpr (std::is_same_v<T, seconds>)
            qDebug("%s\n-> %1.3f (avg %1.3f) seconds", stringView.data(), timeout.count(), avg / ctr);
        else if constexpr (std::is_same_v<T, minutes>)
            qDebug("%s\n-> %1.3f (avg %1.3f) minutes", stringView.data(), timeout.count(), avg / ctr);
        else if constexpr (std::is_same_v<T, hours>)
            qDebug("%s\n-> %1.3f (avg %1.3f) hours", stringView.data(), timeout.count(), avg / ctr);
    }
};
template <class T>
Timer(std::string_view, T) -> Timer<T>;

inline auto to_sv16(const QString& str)
{
    return std::u16string_view(reinterpret_cast<const char16_t*>(str.utf16()), str.size());
}

template <class T>
struct rxCap {
    T& cap;
    rxCap(T& cap)
        : cap { cap }
    {
    }
    operator QString() const { return QString(reinterpret_cast<const QChar*>(cap.data()), cap.size()); }
    auto toInt() const { return QString(reinterpret_cast<const QChar*>(cap.data()), cap.size()).toInt(); }
    auto toDouble() const { return QString(reinterpret_cast<const QChar*>(cap.data()), cap.size()).toDouble(); }

    operator int() const { return toInt(); }
    operator double() const { return toDouble(); }
};
template <class T>
rxCap(T) -> rxCap<T>;
