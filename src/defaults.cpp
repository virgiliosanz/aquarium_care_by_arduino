#include <defaults.h>

namespace defaults {

word HourMinute::minutes() const {
	word m = (hour * 60) + minute;
	return m;
}

bool Period::in_period(const HourMinute& hm) const {
	word m = hm.minutes();
	return (init.minutes() <= m && m <= end.minutes());
}

} // namespace defaults
