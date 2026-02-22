#ifndef LIBFLAC_H
#define LIBFLAC_H

#include "FLAC/stream_decoder.h"
#include "FLAC/stream_encoder.h"
#include "FLAC/metadata.h"

class CLibFlac
{
public:
	CLibFlac() {};
	~CLibFlac() {};

	// --- Decoder lifecycle ---

	FLAC__StreamDecoder* stream_decoder_new()
	{
		return ::FLAC__stream_decoder_new();
	}

	void stream_decoder_delete(FLAC__StreamDecoder *decoder)
	{
		::FLAC__stream_decoder_delete(decoder);
	}

	FLAC__StreamDecoderInitStatus stream_decoder_init_stream(
		FLAC__StreamDecoder *decoder,
		FLAC__StreamDecoderReadCallback read_callback,
		FLAC__StreamDecoderSeekCallback seek_callback,
		FLAC__StreamDecoderTellCallback tell_callback,
		FLAC__StreamDecoderLengthCallback length_callback,
		FLAC__StreamDecoderEofCallback eof_callback,
		FLAC__StreamDecoderWriteCallback write_callback,
		FLAC__StreamDecoderMetadataCallback metadata_callback,
		FLAC__StreamDecoderErrorCallback error_callback,
		void *client_data)
	{
		return ::FLAC__stream_decoder_init_stream(decoder,
			read_callback, seek_callback, tell_callback, length_callback,
			eof_callback, write_callback, metadata_callback, error_callback,
			client_data);
	}

	// --- Decoder settings ---

	FLAC__bool stream_decoder_set_md5_checking(FLAC__StreamDecoder *decoder, FLAC__bool value)
	{
		return ::FLAC__stream_decoder_set_md5_checking(decoder, value);
	}

	FLAC__bool stream_decoder_set_metadata_respond(FLAC__StreamDecoder *decoder, FLAC__MetadataType type)
	{
		return ::FLAC__stream_decoder_set_metadata_respond(decoder, type);
	}

	FLAC__bool stream_decoder_set_metadata_respond_application(FLAC__StreamDecoder *decoder, const FLAC__byte id[4])
	{
		return ::FLAC__stream_decoder_set_metadata_respond_application(decoder, id);
	}

	FLAC__bool stream_decoder_set_metadata_respond_all(FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_set_metadata_respond_all(decoder);
	}

	FLAC__bool stream_decoder_set_metadata_ignore(FLAC__StreamDecoder *decoder, FLAC__MetadataType type)
	{
		return ::FLAC__stream_decoder_set_metadata_ignore(decoder, type);
	}

	FLAC__bool stream_decoder_set_metadata_ignore_application(FLAC__StreamDecoder *decoder, const FLAC__byte id[4])
	{
		return ::FLAC__stream_decoder_set_metadata_ignore_application(decoder, id);
	}

	FLAC__bool stream_decoder_set_metadata_ignore_all(FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_set_metadata_ignore_all(decoder);
	}

	// --- Decoder state getters ---

	FLAC__StreamDecoderState stream_decoder_get_state(const FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_get_state(decoder);
	}

	const char* stream_decoder_get_resolved_state_string(const FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_get_resolved_state_string(decoder);
	}

	FLAC__bool stream_decoder_get_md5_checking(const FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_get_md5_checking(decoder);
	}

	FLAC__uint64 stream_decoder_get_total_samples(const FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_get_total_samples(decoder);
	}

	unsigned stream_decoder_get_channels(const FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_get_channels(decoder);
	}

	FLAC__ChannelAssignment stream_decoder_get_channel_assignment(const FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_get_channel_assignment(decoder);
	}

	unsigned stream_decoder_get_bits_per_sample(const FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_get_bits_per_sample(decoder);
	}

	unsigned stream_decoder_get_sample_rate(const FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_get_sample_rate(decoder);
	}

	unsigned stream_decoder_get_blocksize(const FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_get_blocksize(decoder);
	}

	FLAC__bool stream_decoder_get_decode_position(const FLAC__StreamDecoder *decoder, FLAC__uint64 *position)
	{
		return ::FLAC__stream_decoder_get_decode_position(decoder, position);
	}

	// --- Decoder control ---

	FLAC__bool stream_decoder_finish(FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_finish(decoder);
	}

	FLAC__bool stream_decoder_flush(FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_flush(decoder);
	}

	FLAC__bool stream_decoder_reset(FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_reset(decoder);
	}

	// --- Decoder processing ---

	FLAC__bool stream_decoder_process_single(FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_process_single(decoder);
	}

	FLAC__bool stream_decoder_process_until_end_of_metadata(FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_process_until_end_of_metadata(decoder);
	}

	FLAC__bool stream_decoder_process_until_end_of_stream(FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_process_until_end_of_stream(decoder);
	}

	FLAC__bool stream_decoder_skip_single_frame(FLAC__StreamDecoder *decoder)
	{
		return ::FLAC__stream_decoder_skip_single_frame(decoder);
	}

	FLAC__bool stream_decoder_seek_absolute(FLAC__StreamDecoder *decoder, FLAC__uint64 sample)
	{
		return ::FLAC__stream_decoder_seek_absolute(decoder, sample);
	}
};

/*
 * ============================================================================
 * Quick Reference: Types, Enums, and Functions available after including
 * this header. This is the subset used by the XBMC FLACCodec port, but
 * the full FLAC API is available through the included headers above.
 * ============================================================================
 *
 * --- Basic Types (from FLAC/ordinals.h, via format.h) ---
 *
 *   FLAC__int8, FLAC__uint8
 *   FLAC__int16, FLAC__uint16
 *   FLAC__int32, FLAC__uint32
 *   FLAC__int64, FLAC__uint64
 *   FLAC__bool
 *   FLAC__byte
 *
 * --- Key Structures (from FLAC/format.h) ---
 *
 *   FLAC__Frame              - Decoded audio frame (header + subframes + footer)
 *   FLAC__FrameHeader        - Frame header with blocksize, sample_rate, channels, bits_per_sample
 *   FLAC__StreamMetadata     - Metadata block (polymorphic: stream_info, vorbis_comment, etc.)
 *   FLAC__StreamMetadata_StreamInfo - Stream info: sample_rate, channels, bits_per_sample,
 *                                     total_samples, max_blocksize, etc.
 *
 * --- Decoder Enums (from FLAC/stream_decoder.h) ---
 *
 *   FLAC__StreamDecoderState
 *     FLAC__STREAM_DECODER_SEARCH_FOR_METADATA
 *     FLAC__STREAM_DECODER_END_OF_STREAM
 *     FLAC__STREAM_DECODER_SEEK_ERROR
 *     FLAC__STREAM_DECODER_UNINITIALIZED
 *     ...
 *
 *   FLAC__StreamDecoderInitStatus
 *     FLAC__STREAM_DECODER_INIT_STATUS_OK
 *     ...
 *
 *   FLAC__StreamDecoderReadStatus
 *     FLAC__STREAM_DECODER_READ_STATUS_CONTINUE
 *     FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM
 *     FLAC__STREAM_DECODER_READ_STATUS_ABORT
 *
 *   FLAC__StreamDecoderSeekStatus
 *     FLAC__STREAM_DECODER_SEEK_STATUS_OK
 *     FLAC__STREAM_DECODER_SEEK_STATUS_ERROR
 *
 *   FLAC__StreamDecoderTellStatus
 *     FLAC__STREAM_DECODER_TELL_STATUS_OK
 *     FLAC__STREAM_DECODER_TELL_STATUS_ERROR
 *
 *   FLAC__StreamDecoderLengthStatus
 *     FLAC__STREAM_DECODER_LENGTH_STATUS_OK
 *     FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR
 *
 *   FLAC__StreamDecoderWriteStatus
 *     FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE
 *     FLAC__STREAM_DECODER_WRITE_STATUS_ABORT
 *
 *   FLAC__StreamDecoderErrorStatus
 *     FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC
 *     FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER
 *     FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH
 *
 *   FLAC__MetadataType
 *     FLAC__METADATA_TYPE_STREAMINFO
 *     FLAC__METADATA_TYPE_VORBIS_COMMENT
 *     ...
 *
 *   FLAC__ChannelAssignment
 *     FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT
 *     ...
 *
 * --- Decoder Opaque Type ---
 *
 *   FLAC__StreamDecoder
 *
 * --- Callback Typedefs (from FLAC/stream_decoder.h) ---
 *
 *   FLAC__StreamDecoderReadCallback
 *   FLAC__StreamDecoderSeekCallback
 *   FLAC__StreamDecoderTellCallback
 *   FLAC__StreamDecoderLengthCallback
 *   FLAC__StreamDecoderEofCallback
 *   FLAC__StreamDecoderWriteCallback
 *   FLAC__StreamDecoderMetadataCallback
 *   FLAC__StreamDecoderErrorCallback
 *
 * --- Stream Decoder Functions (from FLAC/stream_decoder.h) ---
 *   All declared with FLAC_API linkage (resolves to nothing for static lib).
 *
 *   FLAC__stream_decoder_new()
 *   FLAC__stream_decoder_delete()
 *   FLAC__stream_decoder_init_stream()
 *   FLAC__stream_decoder_set_md5_checking()
 *   FLAC__stream_decoder_set_metadata_respond()
 *   FLAC__stream_decoder_set_metadata_respond_application()
 *   FLAC__stream_decoder_set_metadata_respond_all()
 *   FLAC__stream_decoder_set_metadata_ignore()
 *   FLAC__stream_decoder_set_metadata_ignore_application()
 *   FLAC__stream_decoder_set_metadata_ignore_all()
 *   FLAC__stream_decoder_get_state()
 *   FLAC__stream_decoder_get_resolved_state_string()
 *   FLAC__stream_decoder_get_md5_checking()
 *   FLAC__stream_decoder_get_total_samples()
 *   FLAC__stream_decoder_get_channels()
 *   FLAC__stream_decoder_get_channel_assignment()
 *   FLAC__stream_decoder_get_bits_per_sample()
 *   FLAC__stream_decoder_get_sample_rate()
 *   FLAC__stream_decoder_get_blocksize()
 *   FLAC__stream_decoder_get_decode_position()
 *   FLAC__stream_decoder_finish()
 *   FLAC__stream_decoder_flush()
 *   FLAC__stream_decoder_reset()
 *   FLAC__stream_decoder_process_single()
 *   FLAC__stream_decoder_process_until_end_of_metadata()
 *   FLAC__stream_decoder_process_until_end_of_stream()
 *   FLAC__stream_decoder_skip_single_frame()
 *   FLAC__stream_decoder_seek_absolute()
 *
 * ============================================================================
 */

#endif /* LIBFLAC_H */
