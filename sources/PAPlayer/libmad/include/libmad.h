#ifndef LIBMAD_WRAPPER_H
#define LIBMAD_WRAPPER_H

/*
 * CLibMad - Xbox 360 wrapper class for libmad (statically linked).
 *
 * Follows the same pattern as CLibFlac: a thin inline class that forwards
 * calls to the C library functions.  On the original Xbox, MADCodec.dll was
 * loaded dynamically; here we link the static lib directly and expose the
 * API through this wrapper so the calling code (MP3Codec) has a clean
 * object-oriented interface.
 */

#include "mad.h"

class CLibMad
{
public:
	CLibMad()  {}
	~CLibMad() {}

	// ---- Stream ----

	void stream_init(struct mad_stream *stream)
	{
		::mad_stream_init(stream);
	}

	void stream_finish(struct mad_stream *stream)
	{
		::mad_stream_finish(stream);
	}

	void stream_buffer(struct mad_stream *stream,
	                   unsigned char const *buffer, unsigned long length)
	{
		::mad_stream_buffer(stream, buffer, length);
	}

	int stream_sync(struct mad_stream *stream)
	{
		return ::mad_stream_sync(stream);
	}

	char const *stream_errorstr(struct mad_stream const *stream)
	{
		return ::mad_stream_errorstr(stream);
	}

	// ---- Frame ----

	void frame_init(struct mad_frame *frame)
	{
		::mad_frame_init(frame);
	}

	void frame_finish(struct mad_frame *frame)
	{
		::mad_frame_finish(frame);
	}

	int frame_decode(struct mad_frame *frame, struct mad_stream *stream)
	{
		return ::mad_frame_decode(frame, stream);
	}

	void frame_mute(struct mad_frame *frame)
	{
		::mad_frame_mute(frame);
	}

	// ---- Synth ----

	void synth_init(struct mad_synth *synth)
	{
		::mad_synth_init(synth);
	}

	void synth_mute(struct mad_synth *synth)
	{
		::mad_synth_mute(synth);
	}

	void synth_frame(struct mad_synth *synth, struct mad_frame const *frame)
	{
		::mad_synth_frame(synth, frame);
	}

	// ---- Timer ----

	void timer_add(mad_timer_t *timer, mad_timer_t incr)
	{
		::mad_timer_add(timer, incr);
	}

	// ---- Header ----

	int header_decode(struct mad_header *header, struct mad_stream *stream)
	{
		return ::mad_header_decode(header, stream);
	}
};

/*
 * ============================================================================
 * Quick Reference: Types, Enums, and Functions available after including
 * this header (the full libmad API is available through mad.h above).
 * ============================================================================
 *
 * --- Basic Types ---
 *
 *   mad_fixed_t           - 4.28 signed fixed-point
 *   mad_fixed64_t         - signed __int64
 *   mad_sample_t          - alias for mad_fixed_t
 *   mad_timer_t           - seconds + fraction
 *
 * --- Key Structures ---
 *
 *   struct mad_stream     - bitstream state
 *   struct mad_frame      - decoded frame (header + subband samples)
 *   struct mad_header     - layer, mode, bitrate, samplerate, etc.
 *   struct mad_synth      - synthesis filter state + PCM output
 *   struct mad_pcm        - samplerate, channels, length, samples[2][1152]
 *
 * --- Important Macros ---
 *
 *   MAD_RECOVERABLE(error)    - nonzero if the error is recoverable
 *   MAD_NCHANNELS(header)     - 1 or 2
 *   MAD_F_FRACBITS            - 28
 *   MAD_OPTION_IGNORECRC      - stream option flag
 *
 * --- Enums ---
 *
 *   enum mad_error         - error codes (MAD_ERROR_BUFLEN, etc.)
 *   enum mad_layer         - MAD_LAYER_I / II / III
 *   enum mad_mode          - single, dual, joint stereo, stereo
 *
 * --- C Functions (wrapped above) ---
 *
 *   mad_stream_init / finish / buffer / sync / errorstr
 *   mad_frame_init / finish / decode / mute
 *   mad_synth_init / mute / frame
 *   mad_timer_reset / add
 *   mad_header_init / decode
 *
 * ============================================================================
 */

#endif /* LIBMAD_WRAPPER_H */
