#ifndef SHL_PNG_DECODER_H__
#define SHL_PNG_DECODER_H__

#include <stdint.h>
#include <iostream>

#define SHL_PNG_PAGE_SIZE 4096
#define SHL_PNG_MAKE_CHUNK_HEADER(A, B, C, D) ((uint32_t) ((uint32_t)D << 24) | ((uint32_t)C << 16) | ((uint32_t)B << 8) | (uint32_t)A)
#define SHL_PNG_BEGIN_IDAT 4

#define SHL_PNG_ZLIB_COMPRESSION_TYPE_NONE 0 
#define SHL_PNG_ZLIB_COMPRESSION_TYPE_FIXED 1
#define SHL_PNG_ZLIB_COMPRESSION_TYPE_DYNAMIC 2
#define SHL_PNG_ZLIB_COMPRESSION_TYPE_ERROR 3

#define SHL_PNG_CONTINUATION_BIT 0b10000000
#define SHL_PNG_COMPRESSION_TYPE_BITS 0b00000110
#define SHL_PNG_FINAL_BLOCK_BIT 0b00000001
#define SHL_PNG_FIRST_IDAT_CHUNK_BIT 0b01000000
#define SHL_PNG_NEW_IDAT 3


#define SHL_PNG_COLOR_TYPE_GRAY 0
#define SHL_PNG_COLOR_TYPE_RGB 2
#define SHL_PNG_COLOR_TYPE_PALETTE 3
#define SHL_PNG_COLOR_TYPE_GRAY_ALPHA 4
#define SHL_PNG_COLOR_TYPE_RGB_ALPHA 6

#define SHL_PNG_INTERLACE_NONE 0
#define SHL_PNG_INTERLACE_ADAM7 1

#ifdef SHL_PNG_LOG_ERRORS
#define SHL_LOG_WARN
#include "logging.h"
#define SHL_PNG_LOG(...) shl::logWarn(__VA_ARGS__)
#else
#define SHL_PNG_LOG(...)
#endif



namespace shl {
    inline const uint8_t PNG_HEADER[] = { 137, 80, 78, 71, 13, 10, 26, 10 };

	struct PngDecoder {
		char* imageBuf;
		uint32_t imageWidth;
		uint32_t imageHeight;
        size_t imageBytePosition;
		char pageBuf[SHL_PNG_PAGE_SIZE];
	    std::ifstream file;
		size_t fileSize;
#ifdef SHL_PNG_CHECK_CRC
		uint32_t crc;
#endif
		uint32_t chunkType;
		uint32_t chunkSize;
		uint16_t pagePos;
		uint8_t zlibCompressionType;
        uint8_t bitDepthPixel;
        uint8_t colorType;
        uint8_t interlaceMethod;

        size_t getFilePos() {
            return file.tellg();
        }
	};


#ifdef SHL_PNG_CHECK_CRC
    inline uint32_t CRC_TABLE[256] = {};
    void _MakeCRCTable()
    {
        uint32_t c;
        uint32_t n, k;

        for (n = 0; n < 256; n++) {
            c = n;
            for (k = 0; k < 8; k++) {
                if (c & 1)
                    c = 0xedb88320L ^ (c >> 1);
                else
                    c = c >> 1;
            }
            CRC_TABLE[n] = c;
        }
    }
    static inline uint32_t _UpdateCRC(uint32_t crc, const char* buf, uint32_t len)
    {
        uint32_t c = crc;
        for (uint32_t n = 0; n < len; n++) {
            c = CRC_TABLE[(c ^ buf[n]) & 0xff] ^ (c >> 8);
        }
        return c;
    }
#endif

    // returns the next four bytes of the pointer in reversed order as U32
    // requires that the pointer contains at least four bytes
    static inline uint32_t _GetAsU32(const char* four_bytes) {
        char reversed[4] = { four_bytes[3], four_bytes[2], four_bytes[1], four_bytes[0] };
        return *(uint32_t*)&reversed;
    }
    // returns the next two bytes of the pointer in reversed order as U16
    // requires that the pointer contains at least two bytes
    static inline uint16_t _GetAsU16(const char* two_bytes) {
        char reversed[2] = { two_bytes[1], two_bytes[0] };
        return *(uint16_t*)&reversed;
    }

    // reads a page of the buffer and stores it in a buffer
    static inline bool _ReadPngPage(PngDecoder& state) {
        state.pagePos = 0;
    }

    // reads the next byte of the page buffer without checking if the end of the page is reached
    static inline uint8_t _ReadByteUnsafe(PngDecoder& state) {
        return state.pageBuf[state.pagePos++];
    }

    static inline uint8_t _ReadByte(PngDecoder& state) {
        if (state.pagePos >= SHL_PNG_PAGE_SIZE) {
            _ReadPngPage(state);
        }
        return _ReadByteUnsafe(state);
    }

    static inline void _IncrementFilePointer(PngDecoder& state) {
        if (state.pagePos < (SHL_PNG_PAGE_SIZE - 1)) {
			state.pagePos++;
		}
        else {
			state.pagePos = 0;
            _ReadPngPage(state);
        }
    }

    static inline void _MoveFilePointer(PngDecoder& state, uint_fast16_t amount) {
        if ((state.pagePos + amount) < SHL_PNG_PAGE_SIZE) {
			state.pagePos += amount;
        }
        else {
            state.pagePos = (state.pagePos + amount) - SHL_PNG_PAGE_SIZE;
			_ReadPngPage(state);
        }
    }

    static inline uint32_t _ReadU32(PngDecoder& state) {
        uint8_t buf[4];
        for (uint_fast8_t i = 0; i < 4; i++)
        {
            
        }
    }
    // initializes the PngState object and returns true if successful
	// checks wether the opened file is a valid png-file
    static inline bool _InitPngState(PngDecoder& state, const char* filename) {
        state.file.open(filename, std::ios::binary | std::ios::ate);
        if (!state.file.is_open()) {
            SHL_PNG_LOG("failed to open file: ", filename);
            return false;
        }
        state.fileSize = state.file.tellg();
        state.file.seekg(0, std::ios::beg);
        _pngStateReadBuf(state);
        uint64_t& magic_number = *(uint64_t*)state.fileBuf[0];
        uint64_t& png_id = *(uint64_t*)&PNG_HEADER[0];
        uint32_t& chunk_header = *(uint32_t*)state.fileBuf[8];
        if (state.fileSize < 57 || magic_number != png_id 
            || chunk_header != SHL_PNG_MAKE_CHUNK_HEADER('I', 'H', 'D', 'R')) {
            return false;
        }
        uint32_t chunk_size = _getAsU32(state.fileBuf[12]);
        state.width = _GetAsU32(state.fileBuf[16]);
        state.height = _GetAsU32(state.fileBuf[20]);
		state.bitDepth = state.fileBuf[24];
        state.colorType = state.fileBuf[25];
        state.compressionType = 0; // compression type of the file not nessesary for decoding 
        state.filterType = state.fileBuf[27];
		state.interlaceMethod = state.fileBuf[28];
#ifdef SHL_PNG_CHECK_CRC
        _MakeCRCTable();
        uint32_t calculated_crc = _UpdateCRC(0xFFFFFFFF, state.fileBuf[12], 17) ^ 0xFFFFFFFF;
        if (calculated_crc != _GetAsU32(state.fileBuf[29])) {
            return false;
        }
#endif
        state.pagePos = 33;
        state.bitPos = 0;
        state.bufferPos = 0; 
        state.imageBytePosition = 0;
        return true;
    }
    static inline void _ParseChunkHeader(PngDecoder& state) {
        if (state.pagePos < SHL_PNG_PAGE_SIZE - 8) {
            state.chunkType = *(uint32_t*)state.pageBuf[state.pagePos];
            state.chunkSize = _getAsU32(state.pageBuf[state.pagePos + 4]);
            state.pagePos += 8;
#ifdef SHL_PNG_CHECK_CRC
            state.crc = _UpdateCRC(0xFFFFFFFF, state.pageBuf[state.pagePos], 8);
#endif
        }
        else {
            char temp[8];
            uint16_t diff = SHL_PNG_PAGE_SIZE - state.pagePos;
            memcpy(temp, state.fileBuf[state.pagePos], diff);
#ifdef SHL_PNG_CHECK_CRC
            state.crc = _UpdateCRC(0xFFFFFFFF, state.pageBuf[state.pagePos], diff);
#endif
            _ReadPngPage(state);
            state.pagePos = 8 - diff;
#ifdef SHL_PNG_CHECK_CRC
            state.crc = _updateCRC(0xFFFFFFFF, state.fileBuf, state.pagePos);
#endif
            memcpy(&temp[diff], state.fileBuf, state.pagePos);
            state.chunkType = *(uint32_t*)&temp[0];
            state.chunkSize = _GetAsU32(&temp[4]);
        }
    }
    static inline void _WriteByteToBuffer(const char byte, PngDecoder& state) {
		buffer[state.bufferPos] = byte;
        state.bufferPos++;
    }
    // loads pixels into existing buffer and sets the loaded height and width.
    // logs a warning if buffer size is smaller than the required size.
    // sets the height and width to the dimensions read.
    static inline bool loadImageIntoBuffer(const char* filename, void* buffer, size_t buffer_size, uint32_t* pWidth, uint32_t* pHeight) {
        PngDecoder state;
        if (!_InitPngState(state, filename)) {
            return false;
        }
        do {
            _ParseChunkHeader(state);
            switch (state.chunkType) {
            case (SHL_PNG_MAKE_CHUNK_HEADER('I', 'D', 'A', 'T')): {
                uint32_t bytes_read = state.pagePos;
                if (state.compressionType == SHL_PNG_BEGIN_IDAT) { // signals that this is the firt idat chunk
                    // skipping the header of the zlib data stream
					_pngStateIncrease(state, 2); 
                    // compression type reserved
                    state.compressionType = 3; 
                }
                if (state.compressionType == SHL_PNG_NEW_IDAT) { 
                    // new chunk
                    char block_header = _getCurrentByte(state);
                    state.compressionType = block_header & 6;
                    if (block_header & 1) {
                        state.compressionType = state.compressionType | 1;
                    }
                }
                // no compression
                if ((state.compressionType & 3) == SHL_PNG_ZLIB_COMPRESSION_TYPE_NO_COMPRESSION) {
                    _pngStateIncrement(state);
                    uint16_t length = _readNextU16(state);
					_pngStateIncrease(state, 2);
					for (uint16_t i = 0; i < length; ++i) {
                        const char byte = _readNextByte(state);
                        _writeByteToBuffer(byte, state, (char*)buffer);
					}
                }
				else {
                    HuffmanTree huffman_tree;
                    if ((state.compressionType & 3) == SHL_PNG_ZLIB_COMPRESSION_TYPE_DYNAMIC) {
                        huffman_tree = _getDynamicHuffmanTree(state);
                    }
                    else {
                        huffman_tree = _getStaticHuffmanTree();
                    }

                    bool end_of_block = false;
                    do {

                    } while (bytes_read < state.chunkSize);
                }
                
                
			} break;
            case (SHL_PNG_MAKE_CHUNK_HEADER('I', 'E', 'N', 'D')): {
            } break;
            case (SHL_PNG_MAKE_CHUNK_HEADER('P', 'T', 'L', 'E')): {
            } break;
            default: {
                _pngStateIncrease(state, state.chunkSize);
            } break;
            }
			_pngStateIncrease(state, state.chunkSize);
#ifdef SHL_PNG_CHECK_CRC
			state.crc = _updateCRC(state.crc, state.fileBuf[state.pagePos], state.chunkSize);
			uint32_t chunk_crc = _readNextU32(state);
			if (state.crc != chunk_crc) {
				return false;
			}
#else
            _pngStateIncrease(state, 4);
#endif
        } while (state.chunkType != SHL_PNG_MAKE_CHUNK_HEADER('I', 'E', 'N', 'D'));
    }

}
#endif