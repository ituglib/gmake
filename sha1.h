#if ! defined(_SHA1_HEADER_H_)
#define _SHA1_HEADER_H_
/*
 * SHA1 routine optimized to do word accesses rather than byte accesses,
 * and to avoid unnecessary copies into the context array.
 *
 * This was initially based on the Mozilla SHA1 implementation, although
 * none of the original Mozilla code remains. The code is based on Git's sha1.c and subject
 * to Git's GNUv2 Public License.
 */

/**
 * Context for the SHA-1 calculation.
 */
typedef struct {
	/** The size of the buffer */
	unsigned long long size;
	/** The current rotation context */
	unsigned int H[5];
	/** The current rotation value. */
	unsigned int W[16];
} blk_SHA_CTX;

/**
 * Initialize the SHA1 calculation.
 * @param ctx
 *     the SHA1 context buffer to initialize.
 */
extern void blk_SHA1_Init(blk_SHA_CTX *ctx);
/**
 * Update the SHA1 buffer with content.
 * @param ctx
 *     the SHA1 context buffer to initialize.
 * @param dataIn
 *     the input buffer to add to the SHA1 calculation.
 * @param len
 *     the length of <b>dataIn</b>.
 */
extern void blk_SHA1_Update(blk_SHA_CTX *ctx, const void *dataIn, unsigned long len);
/**
 * Finalize the SHA1 buffer.
 * @param ctx
 *     the SHA1 context buffer to finalize.
 * @param hashout
 *     the SHA1 result in binary.
 */
extern void blk_SHA1_Final(unsigned char hashout[20], blk_SHA_CTX *ctx);
/**
 * Convert the SHA1 result to ASCII.
 * @param hashout
 *     the SHA1 result in binary.
 * @param buffer
 *     the SHA1 result in ASCII.
 */
extern void blk_SHA1_ToAscii(unsigned char hashout[20], char *buffer);
#endif
