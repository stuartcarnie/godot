#include <memory>

/** Selects and returns one of the values, based on the platform OS. */
template <typename T>
const T &mvkSelectPlatformValue(const T &macOSVal, const T &iOSVal) {
#if (TARGET_OS_IOS || TARGET_OS_TV) && !TARGET_OS_MACCATALYST
	return iOSVal;
#endif
#if TARGET_OS_OSX
	return macOSVal;
#endif
}

#pragma mark - Values and Structs

/**
 * If pVal is not null, clears the memory occupied by *pVal by writing zeros to all bytes.
 * The optional count allows clearing multiple elements in an array.
 */
template <typename T>
void mvkClear(T *pVal, size_t count = 1) {
	if (pVal) {
		memset(pVal, 0, sizeof(T) * count);
	}
}

/**
 * If pV1 and pV2 are both not null, returns whether the contents of the two values are equal,
 * otherwise returns false. The optional count allows comparing multiple elements in an array.
 */
template<typename T>
bool mvkAreEqual(const T* pV1, const T* pV2, size_t count = 1) {
	return (pV1 && pV2) && (memcmp(pV1, pV2, sizeof(T) * count) == 0);
}

#pragma mark - Boolean flags

/** Enables the flags (sets bits to 1) within the value parameter specified by the bitMask parameter. */
template <typename Tv, typename Tm>
void mvkEnableFlags(Tv &value, const Tm bitMask) { value = (Tv)(value | bitMask); }

/** Disables the flags (sets bits to 0) within the value parameter specified by the bitMask parameter. */
template <typename Tv, typename Tm>
void mvkDisableFlags(Tv &value, const Tm bitMask) { value = (Tv)(value & ~(Tv)bitMask); }

/** Returns whether the specified value has ANY of the flags specified in bitMask enabled (set to 1). */
template<typename Tv, typename Tm>
static constexpr bool mvkIsAnyFlagEnabled(Tv value, const Tm bitMask) { return ((value & bitMask) != 0); }

/** Returns whether the specified value has ALL of the flags specified in bitMask enabled (set to 1). */
template <typename Tv, typename Tm>
static constexpr bool mvkAreAllFlagsEnabled(Tv value, const Tm bitMask) { return ((value & bitMask) == bitMask); }

#pragma mark - Math

/** Returns the result of a division, rounded up. */
template <typename T, typename U>
constexpr typename std::common_type<T, U>::type mvkCeilingDivide(T numerator, U denominator) {
	typedef typename std::common_type<T, U>::type R;
	// Short circuit very common usecase of dividing by one.
	return (denominator == 1) ? numerator : (R(numerator) + denominator - 1) / denominator;
}
