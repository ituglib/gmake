#ifndef _MAKE_TANDEM_DEFS_H_
#define _MAKE_TANDEM_DEFS_H_

/**
 * Positioning modes for FILE_SETKEY_ and KEYPOSITION
 */
typedef enum EnscribePositioningMode_ {
	/** Approximate positioning. */
	ENSCRIBE_POSITION_APPROX = 0,
	/** Generic (part key) positioning. */
	ENSCRIBE_POSITION_GENERIC = 1,
	/** Exact positioning. */
	ENSCRIBE_POSITION_EXACT = 2,
} EnscribePositioningMode;

#endif /* _MAKE_TANDEM_DEFS_H_ */
