#pragma once

// Standard libraries

// Third-party libraries
#include <mpack/mpack.h>
#include <etl/cstring.h>
#include <etl/type_traits.h>
#include <etl/flat_map.h>
#include <etl/bitset.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Util/Pair.h"
#include "Phyllo/Protocol/Types.h"

// MessagePack format for document serialization and deserialization.

namespace Phyllo { namespace Protocol { namespace Presentation {

namespace MsgPack {
  static const SerializationFormatCode kFormat = SerializationFormat::Binary::Dynamic::MsgPack;

  enum class FieldType {
    Missing = mpack_type_missing,
    None = mpack_type_nil,
    Boolean = mpack_type_bool,
    Uint = mpack_type_uint,
    Int = mpack_type_int,
    Float32 = mpack_type_float,
    Float64 = mpack_type_double,
    String = mpack_type_str,
    Binary = mpack_type_bin,
    Array = mpack_type_array,
    Map = mpack_type_map
  };
}

template<>
class DocumentReader<MsgPack::kFormat> {
  public:
    mpack_reader_t reader;

    DocumentReader(
      const ByteBuffer &buffer,
      size_t bufferStartOffset = 0, // absolute offset from start of buffer to when the reader can start reading
      size_t bufferEndOffset = 0 // absolute offset from end of buffer to when the reader must stop reading
    ) :
      buffer(buffer),
      startOffset(bufferStartOffset), endOffset(bufferEndOffset) {}

    // Core reader methods

    void start() {
      mpack_reader_init_data(
        &reader, 
        reinterpret_cast<const char *>(buffer.data() + startOffset), bufferSize()
      );
    }

    bool finish() {
      return error() == mpack_ok;
    }

    ByteBufferView bufferRemaining() const {
      const char *remaining = nullptr;
      size_t bytesRemaining = mpack_reader_remaining(const_cast<mpack_reader_t *>(&reader), &remaining);
      return ByteBufferView(buffer.end() - endOffset - bytesRemaining, buffer.end());
    }

    size_t bufferSize() const {
      return buffer.size() - startOffset - endOffset;
    }

    void flagError(mpack_error_t error) { mpack_reader_flag_error(&reader, error); }

    mpack_error_t flagIfError(mpack_error_t error) {
      return mpack_reader_flag_if_error(&reader, error);
    }

    mpack_error_t error() const {
      return mpack_reader_error(const_cast<mpack_reader_t *>(&reader));
    }

    // Type-specified reads

    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, None>::value, Type>::type
    read() { return readNone(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, bool>::value, Type>::type
    read() { return readBoolean(); }
    #if PHYLLO_PLATFORM != PHYLLO_PLATFORM_ATMELAVR
    // unsigned int is just uint16_t on AVR, which leads to conflict in templates
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, unsigned int>::value, Type>::type
    read() { return readUint(); }
    #endif
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, uint8_t>::value, Type>::type
    read() { return readUint8(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, uint16_t>::value, Type>::type
    read() { return readUint16(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, uint32_t>::value, Type>::type
    read() { return readUint32(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, uint64_t>::value, Type>::type
    read() { return readUint64(); }
    #if PHYLLO_PLATFORM != PHYLLO_PLATFORM_ATMELAVR
    // unsigned int is just uint16_t on AVR, which leads to conflict in templates
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, int>::value, Type>::type
    read() { return readInt(); }
    #endif
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, int8_t>::value, Type>::type
    read() { return readInt8(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, int16_t>::value, Type>::type
    read() { return readInt16(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, int32_t>::value, Type>::type
    read() { return readInt32(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, int64_t>::value, Type>::type
    read() { return readInt64(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, float>::value, Type>::type
    read() { return readFloat32(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, double>::value, Type>::type
    read() { return readFloat64(); }


    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, StringView>::value, Type>::type
    read() { return readStringView(); }
    size_t read(String &string) { return readString(string); }
    template<typename Type, size_t Size>
    typename etl::enable_if<etl::is_same<Type, FixedString<Size>>::value, Type>::type
    read() { return readString<Size>(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, String8>::value, Type>::type
    read() { return readString8(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, String16>::value, Type>::type
    read() { return readString16(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, String32>::value, Type>::type
    read() { return readString32(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, String64>::value, Type>::type
    read() { return readString64(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, BinaryView>::value, Type>::type
    read() { return readBinaryView(); }
    size_t read(Binary &buffer) { return readBinary(buffer); }
    template<typename Type, size_t Size>
    typename etl::enable_if<etl::is_same<Type, FixedBinary<Size>>::value, Type>::type
    read() { return readBinary<Size>(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, Binary8>::value, Type>::type
    read() { return readBinary8(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, Binary16>::value, Type>::type
    read() { return readBinary16(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, Binary32>::value, Type>::type
    read() { return readBinary32(); }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, Binary64>::value, Type>::type
    read() { return readBinary64(); }

    template<typename Type>
    size_t read(etl::ivector<Type> &array) { return readArray(array); }
    template<typename Type, size_t Size>
    etl::vector<Type, Size> read() { return readArray<Type, Size>(); }

    template<typename Key, typename Value>
    size_t read(etl::iflat_map<Key, Value> &map) { return readMap(map); }
    template<typename Key, typename Value, size_t Size>
    etl::flat_map<Key, Value, Size> read() { return readMap<Key, Value, Size>(); }

    template<typename Class>
    void read(Class &instance) { return readClass(instance); }
    template<typename Class>
    typename etl::enable_if<!etl::is_one_of<Class, PHYLLO_GENERIC_TYPES>::value, Class>::type
    read() { return readClass<Class>(); }

    // Primitives

    bool isNone() const {
      return peekTagType() == mpack_type_nil;
    }
    None readNone() {
      mpack_expect_nil(&reader);
      return kNone;
    }

    bool isBoolean() const {
      return peekTagType() == mpack_type_bool;
    }
    bool readBoolean() { return mpack_expect_bool(&reader); }
    void readTrue() { mpack_expect_true(&reader); }
    void readFalse() { mpack_expect_false(&reader); }

    bool isIntegral() const {
      return peekTagType() == mpack_type_uint || peekTagType() == mpack_type_int;
    }
    bool isUint() const {
      return peekTagType() == mpack_type_uint;
    }
    unsigned int readUint() {return mpack_expect_uint(&reader); }
    unsigned int readUintRange(unsigned int minValue, unsigned int maxValue) { return mpack_expect_uint_range(&reader, minValue, maxValue); }
    unsigned int readUintMax(unsigned int maxValue) { return mpack_expect_uint_max(&reader, maxValue); }
    void readUintExact(unsigned int value) { return mpack_expect_uint_match(&reader, value); }

    uint8_t readUint8() { return mpack_expect_u8(&reader); }
    uint8_t readUint8Range(uint8_t minValue, uint8_t maxValue) { return mpack_expect_u8_range(&reader, minValue, maxValue); }
    uint8_t readUint8Max(uint8_t maxValue) { return mpack_expect_u8_max(&reader, maxValue); }

    uint16_t readUint16() { return mpack_expect_u16(&reader); }
    uint16_t readUint16Range(uint16_t minValue, uint16_t maxValue) { return mpack_expect_u16_range(&reader, minValue, maxValue); }
    uint16_t readUint16Max(uint16_t maxValue) { return mpack_expect_u16_max(&reader, maxValue); }

    uint32_t readUint32() { return mpack_expect_u32(&reader); }
    uint32_t readUint32Range(uint32_t minValue, uint32_t maxValue) { return mpack_expect_u32_range(&reader, minValue, maxValue); }
    uint32_t readUint32Max(uint32_t maxValue) { return mpack_expect_u32_max(&reader, maxValue); }

    uint64_t readUint64() { return mpack_expect_u64(&reader); }
    uint64_t readUint64Range(uint64_t minValue, uint64_t maxValue) { return mpack_expect_u64_range(&reader, minValue, maxValue); }
    uint64_t readUint64Max(uint64_t maxValue) { return mpack_expect_u64_max(&reader, maxValue); }

    bool isInt() const {
      return peekTagType() == mpack_type_int;
    }
    int readInt() {return mpack_expect_int(&reader); }
    int readIntRange(int minValue, int maxValue) { return mpack_expect_int_range(&reader, minValue, maxValue); }
    int readIntMax(int maxValue) { return mpack_expect_int_max(&reader, maxValue); }
    void readIntExact(int value) { return mpack_expect_int_match(&reader, value); }

    int8_t readInt8() { return mpack_expect_i8(&reader); }
    int8_t readInt8Range(int8_t minValue, int8_t maxValue) { return mpack_expect_i8_range(&reader, minValue, maxValue); }
    int8_t readInt8Max(int8_t maxValue) { return mpack_expect_i8_max(&reader, maxValue); }

    int16_t readInt16() { return mpack_expect_i16(&reader); }
    int16_t readInt16Range(int16_t minValue, int16_t maxValue) { return mpack_expect_i16_range(&reader, minValue, maxValue); }
    int16_t readInt16Max(int16_t maxValue) { return mpack_expect_i16_max(&reader, maxValue); }

    int32_t readInt32() { return mpack_expect_i32(&reader); }
    int32_t readInt32Range(int32_t minValue, int32_t maxValue) { return mpack_expect_i32_range(&reader, minValue, maxValue); }
    int32_t readInt32Max(int32_t maxValue) { return mpack_expect_i32_max(&reader, maxValue); }

    int64_t readInt64() { return mpack_expect_i64(&reader); }
    int64_t readInt64Range(int64_t minValue, int64_t maxValue) { return mpack_expect_i64_range(&reader, minValue, maxValue); }
    int64_t readInt64Max(int64_t maxValue) { return mpack_expect_i64_max(&reader, maxValue); }

    bool isFloat() const {
      return peekTagType() == mpack_type_float || peekTagType() == mpack_type_double;
    }
    bool isFloat32() const {
      return peekTagType() == mpack_type_float;
    }
    float readFloat32() { return mpack_expect_float(&reader); }
    float readFloat32Strict() { return mpack_expect_float_strict(&reader); }
    float readFloat32Range(float minValue, float maxValue) { return mpack_expect_float_range(&reader, minValue, maxValue); }

    bool isFloat64() const {
      return peekTagType() == mpack_type_double;
    }
    double readFloat64() { return mpack_expect_double(&reader); }
    #if MPACK_DOUBLES
    double readFloat64Strict() { return mpack_expect_double_strict(&reader); }
    #endif
    float readFloat64Range(double minValue, double maxValue) { return mpack_expect_double_range(&reader, minValue, maxValue); }

    // Sequences

    bool isString() const {
      return peekTagType() == mpack_type_str;
    }
    uint32_t stringLength() const {
      mpack_tag_t tag = peekTag();
      return mpack_tag_str_length(&tag);
    }

    StringView readStringView() {
      size_t stringLength = startStringLengthMax(bufferSize());
      StringView string = readChunk<StringView>(stringLength);
      finishString();
      return string;
    }
    String8 readString8() { return readString<8>(); }
    String16 readString16() { return readString<16>(); }
    String32 readString32() { return readString<32>(); }
    String64 readString64() { return readString<64>(); }
    template<size_t Size>
    FixedString<Size> readString() {
      FixedString<Size> string;
      readString(string);
      return string;
    }
    size_t readString(String &string) {
      string.resize(string.max_size());
      size_t length = mpack_expect_utf8(&reader, string.data(), string.max_size());
      string.resize(length);
      return length;
    }
    size_t readString(char *buffer, size_t bufferLength) { return mpack_expect_utf8(&reader, buffer, bufferLength); }
    size_t readCString(String &string) {
      string.resize(string.max_size());
      mpack_expect_cstr(&reader, string.data(), string.max_size());
      size_t length = string.find_first_of('\0');
      string.resize(length);
      return length;
    }
    void readCString(char *buffer, size_t bufferLength) { mpack_expect_utf8_cstr(&reader, buffer, bufferLength); }

    void readStringContent(const String &string) {
      readStringContent(StringView(string));
    }
    void readStringContent(const StringView &string) {
      readStringContent(string.data(), string.size());
    }
    void readStringContent(const char *string, size_t length) { return mpack_expect_str_match(&reader, string, length); }
    void readCStringContent(const char *string) { return mpack_expect_cstr_match(&reader, string); }

    uint32_t startString() { return mpack_expect_str(&reader); }
    void startStringLength(uint32_t length) { mpack_expect_str_length(&reader, length); }
    uint32_t startStringLengthMax(uint32_t maxLength) { return mpack_expect_str_max(&reader, maxLength); }
    void finishString() { mpack_done_str(&reader); }

    bool isBinary() const {
      return peekTagType() == mpack_type_bin;
    }
    uint32_t binaryLength() const {
      mpack_tag_t tag = peekTag();
      return mpack_tag_bin_length(&tag);
    }

    BinaryView readBinaryView() {
      size_t bufferLength = startBinaryLengthMax(bufferSize());
      BinaryView buffer = readChunk<BinaryView>(bufferLength);
      finishBinary();
      return buffer;
    }
    Binary8 readBinary8() { return readBinary<8>(); }
    Binary16 readBinary16() { return readBinary<16>(); }
    Binary32 readBinary32() { return readBinary<32>(); }
    Binary64 readBinary64() { return readBinary<64>(); }
    template<size_t Size>
    FixedBinary<Size> readBinary() {
      FixedBinary<Size> buffer;
      readBinary(buffer);
      return buffer;
    }
    size_t readBinary(Binary &buffer) {
      buffer.resize(buffer.max_size());
      size_t length = readBinary(buffer.data(), buffer.size());
      buffer.resize(length);
      return length;
    }
    size_t readBinary(uint8_t *buffer, size_t length) {
      return mpack_expect_bin_buf(&reader, reinterpret_cast<char *>(buffer), length);
    }

    uint32_t startBinary() { return mpack_expect_bin(&reader); }
    void startBinaryLength(uint32_t length) { mpack_expect_bin_size(&reader, length); }
    uint32_t startBinaryLengthMax(uint32_t maxLength) { return mpack_expect_bin_max(&reader, maxLength); }
    void finishBinary() { mpack_done_bin(&reader); }

    void skipBytes(size_t skip) { mpack_skip_bytes(&reader, skip); }

    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, StringView>::value, Type>::type
    readChunk(size_t length) {
      return StringView(mpack_read_utf8_inplace(&reader, length), length);
    }
    template<size_t Size>
    FixedBinary<Size> readChunk() {
      FixedBinary<Size> chunk;
      readChunk(chunk);
      return chunk;
    }
    void readChunk(Binary &chunk) {
      mpack_read_bytes(&reader, reinterpret_cast<char *>(chunk.data()), chunk.max_size());
    }
    template<typename Type>
    typename etl::enable_if<etl::is_same<Type, BinaryView>::value, Type>::type
    readChunk(size_t length) {
      return BinaryView(reinterpret_cast<const uint8_t *>(mpack_read_bytes_inplace(&reader, length)), length);
    }

    // Arrays

    bool isArray() const {
      return peekTagType() == mpack_type_array;
    }
    uint32_t arrayLength() const {
      mpack_tag_t tag = peekTag();
      return mpack_tag_array_count(&tag);
    }

    template<typename Value, size_t Size>
    etl::vector<Value, Size> readArray() {
      etl::vector<Value, Size> array;
      readArray(array);
      return array;
    }
    template<typename Value>
    size_t readArray(etl::ivector<Value> &array) {
      size_t length = startArrayLengthMax(array.max_size());
      for (size_t i = 0; i < length; ++i) array.push_back(read<Value>());
      finishArray();
      return length;
    }

    uint32_t startArray() { return startArrayLengthMax(bufferSize()); }
    void startArrayLength(uint32_t length) { mpack_expect_array_match(&reader, length); }
    uint32_t startArrayLengthMax(uint32_t maxLength) { return mpack_expect_array_max(&reader, maxLength); }
    uint32_t startArrayLengthRange(uint32_t minLength, uint32_t maxLength) { return mpack_expect_array_range(&reader, minLength, maxLength); }
    void finishArray() { mpack_done_array(&reader); }

    // Maps

    bool isMap() const {
      return peekTagType() == mpack_type_map;
    }
    uint32_t mapLength() const {
      mpack_tag_t tag = peekTag();
      return mpack_tag_map_count(&tag);
    }

    template<typename Key, typename Value, size_t Size>
    etl::flat_map<Key, Value, Size> readMap() {
      etl::flat_map<Key, Value, Size> map;
      readMap(map);
      return map;
    }
    template<typename Key, typename Value>
    void readMap(etl::iflat_map<Key, Value> &map) {
      size_t length = startMapLengthMax(map.max_size());
      for (size_t i = 0; i < length; ++i) {
        Key key = read<Key>();
        Value value = read<Value>();
        map[key] = value;
      }
      finishMap();
      return map;
    }

    template<typename Key, typename Value>
    Util::Pair<Key, Value> readPair() {
      Util::Pair<Key, Value> pair;
      pair.first = read<Key>();
      pair.second = read<Value>();
      return pair;
    }

    uint32_t startMap() { return startMapLengthMax(bufferSize() / 2); }
    void startMapLength(uint32_t length) { mpack_expect_map_match(&reader, length); }
    uint32_t startMapLengthMax(uint32_t maxLength) { return mpack_expect_map_max(&reader, maxLength); }
    uint32_t startMapLengthRange(uint32_t minLength, uint32_t maxLength) { return mpack_expect_map_range(&reader, minLength, maxLength); }
    void finishMap() { mpack_done_map(&reader); }

    size_t readKeyCStringIndex(const char *keys[], etl::ibitset &found) {
      size_t count = found.size();
      size_t i = mpack_expect_enum_optional(&reader, keys, count);
      if (i == count) return count;

      // check if this key is a duplicate
      if (found[i]) {
        mpack_reader_flag_error(&reader, mpack_error_invalid);
        return count;
      }

      found[i] = true;
      return i;
    }
    size_t readKeyCStringIndex(const char *keys[], bool found[], size_t count) { return mpack_expect_key_cstr(&reader, keys, found, count); }
    size_t readKeyUint(etl::ibitset &found) {
      size_t count = found.size();
      if (mpack_reader_error(&reader) != mpack_ok) return count;

      if (count == 0) {
        mpack_break("count cannot be zero; no keys are valid!");
        mpack_reader_flag_error(&reader, mpack_error_bug);
        return count;
      }

      // the key is only recognized if it is an unsigned int
      if (!isUint()) {
        discard();
        return count;
      }

      uint64_t value = readUint64();
      if (error() != mpack_ok) return count;

      if (value >= count) return count;

      // check if this key is a duplicate
      if (found[value]) {
        flagError(mpack_error_invalid);
        return count;
      }

      found[value] = true;
      return static_cast<size_t>(value);
    }
    size_t readKeyUint(bool found[], size_t count) { return mpack_expect_key_uint(&reader, found, count); }

    // Other

    None discard() {
      mpack_discard(&reader);
      return kNone;
    }

    MsgPack::FieldType peekType() const {
      return static_cast<MsgPack::FieldType>(peekTagType());
    }

    template<typename Class>
    Class readClass() {
      Class instance;
      readClass(instance);
      return instance;
    }
    template<typename Class>
    bool readClass(Class &instance) {
      return instance.read(*this);
    }

  protected:
    const ByteBuffer &buffer;
    const size_t startOffset;
    const size_t endOffset;

    mpack_tag_t peekTag() const {
      return mpack_peek_tag(const_cast<mpack_reader_t *>(&reader));
    }
    mpack_type_t peekTagType() const {
      mpack_tag_t tag = peekTag();
      return mpack_tag_type(&tag);
    }
};

template<>
class DocumentWriter<MsgPack::kFormat> {
  public:
    mpack_writer_t writer;

    DocumentWriter(
      ByteBuffer &buffer, const DocumentHeader &header,
      size_t bufferStartOffset = 0, // absolute offset from start of buffer to when the writer can start writing
      size_t bufferEndOffset = 0 // absolute offset from end of buffer to when the writer must stop writing
    ) :
      buffer(buffer),
      startOffset(bufferStartOffset), endOffset(bufferEndOffset),
      header(header) {}

    // Core writer methods

    void start() {
      buffer.resize(buffer.max_size());
      mpack_writer_init(
        &writer,
        reinterpret_cast<char *>(buffer.data() + startOffset),
        buffer.max_size() - startOffset - endOffset
      );
    }
    bool finish() {
      buffer.resize(bufferUsed() + startOffset + endOffset);
      return error() == mpack_ok && writeHeader();
    }

    size_t bufferUsed() const {
      return mpack_writer_buffer_used(const_cast<mpack_writer_t *>(&writer));
    }
    size_t bufferAvailable() const {
      return mpack_writer_buffer_left(const_cast<mpack_writer_t *>(&writer));
    }
    size_t bufferSize() const {
      return mpack_writer_buffer_size(const_cast<mpack_writer_t *>(&writer));
    }

    void flagError(mpack_error_t error) { mpack_writer_flag_error(&writer, error); }
    mpack_error_t error() const {
      return mpack_writer_error(const_cast<mpack_writer_t *>(&writer));
    }

    // Type-deduced writes

    void write() { writeNone(); }
    void write(None null) { writeNone(); }
    void write(bool value) { writeBoolean(value); }
    #if PHYLLO_PLATFORM != PHYLLO_PLATFORM_ATMELAVR
    // unsigned int is just uint16_t on AVR, which leads to conflict in templates
    void write(unsigned int number) { writeUint(number); }
    #endif
    void write(uint8_t number) { writeUint8(number); }
    void write(uint16_t number) { writeUint16(number); }
    void write(uint32_t number) { writeUint32(number); }
    void write(uint64_t number) { writeUint64(number); }
    #if PHYLLO_PLATFORM != PHYLLO_PLATFORM_ATMELAVR
    // unsigned int is just uint16_t on AVR, which leads to conflict in templates
    void write(int number) { writeInt(number); }
    #endif
    void write(int8_t number) { writeInt8(number); }
    void write(int16_t number) { writeInt16(number); }
    void write(int32_t number) { writeInt32(number); }
    void write(int64_t number) { writeInt64(number); }
    void write(double number) { writeFloat(number); }

    void write(const String &string) { writeString(StringView(string)); }
    void write(const StringView &string) { writeString(string); }
    void write(const char *string) { writeCString(string); }
    void write(const Binary &buffer) { writeBinary(BinaryView(buffer)); }
    void write(const BinaryView &buffer) { writeBinary(buffer); }

    template<typename... ArgTypes>
    void write(const ArgTypes&... args) { writeTuple(args...); }
    template<typename Type, size_t Size>
    void write(const etl::array<Type, Size> &array) { writeArray(array); }
    template<typename Type>
    void write(const etl::array_view<Type> &array) { writeArray(array); }
    template<typename Type>
    void write(const etl::ivector<Type> &array) { writeArray(array); }

    template<typename Key, typename Value>
    void write(const etl::iflat_map<Key, Value> &map) { writeMap(map); }
    template<typename Key, typename Value>
    void write(const Util::Pair<Key, Value> &pair) { writeKeyValue(pair); }

    template<typename Class>
    typename etl::enable_if<!etl::is_one_of<Class, PHYLLO_GENERIC_TYPES>::value, void>::type
    write(Class &instance) { writeClass(instance); }
    template<typename Class>
    typename etl::enable_if<!etl::is_one_of<Class, PHYLLO_GENERIC_TYPES>::value, void>::type
    write(const Class &instance) { writeClass(instance); }

    // Primitives

    void writeNone() { mpack_write_nil(&writer); }
    void writeNone(None null) { writeNone(); }

    void writeBoolean(bool value) { mpack_write_bool(&writer, value); }

    void writeUint(uint64_t number) { mpack_write_uint(&writer, number); }
    void writeUint8(uint8_t number) { mpack_write_u8(&writer, number); }
    void writeUint16(uint16_t number) { mpack_write_u16(&writer, number); }
    void writeUint32(uint32_t number) { mpack_write_u32(&writer, number); }
    void writeUint64(uint64_t number) { mpack_write_u64(&writer, number); }

    void writeInt(int64_t number) { mpack_write_int(&writer, number); }
    void writeInt8(int8_t number) { mpack_write_i8(&writer, number); }
    void writeInt16(int16_t number) { mpack_write_i16(&writer, number); }
    void writeInt32(int32_t number) { mpack_write_i32(&writer, number); }
    void writeInt64(int64_t number) { mpack_write_i64(&writer, number); }

    void writeFloat(double number) { writeFloat64(number); }
    void writeFloat32(float number) { mpack_write_float(&writer, number); }
    void writeFloat64(double number) { mpack_write_double(&writer, number); }

    // Sequences

    void writeString(const String &string) {
      writeString(StringView(string));
    }
    void writeString(const StringView &string) {
      writeString(string.data(), string.size());
    }
    void writeString(const char *string, uint32_t length) { // We don't have a write override to avoid confusion with the writeTuple write override
      mpack_write_utf8(&writer, string, length);
    }
    void writeCString(const char *string) { mpack_write_utf8_cstr(&writer, string); }

    void writeBinary(const Binary &buffer) {
      writeBinary(BinaryView(buffer));
    }
    void writeBinary(const BinaryView &buffer) {
      writeBinary(buffer.data(), buffer.size());
    }
    void writeBinary(const uint8_t *buffer, uint32_t length) {
      mpack_write_bin(&writer, reinterpret_cast<const char *>(buffer), length);
    }

    void startString(uint32_t length) { mpack_start_str(&writer, length); }
    void finishString() { mpack_finish_str(&writer); }

    void startBinary(uint32_t length) { mpack_start_bin(&writer, length); }
    void finishBinary() { mpack_finish_bin(&writer); }

    void writeChunk(const char *chunk, size_t length) { mpack_write_bytes(&writer, chunk, length); }
    void writeChunk(const String &chunk) {
      writeChunk(StringView(chunk));
    }
    void writeChunk(const StringView &chunk) {
      mpack_write_bytes(&writer, chunk.data(), chunk.size());
    }
    void writeChunk(const Binary &chunk) {
      writeChunk(BinaryView(chunk));
    }
    void writeChunk(const BinaryView &chunk) {
      writeChunk(chunk.data(), chunk.size());
    }
    void writeChunk(const uint8_t *chunk, uint32_t length) {
      mpack_write_bytes(&writer, reinterpret_cast<const char *>(chunk), length);
    }

    // Arrays

    template<typename... ArgTypes>
    void writeTuple(const ArgTypes&... args) {
        startArray(sizeof...(ArgTypes));
        using expand_type = int[];
        expand_type { (write(args), 0)... };
        finishArray();
    }

    template<typename Array>
    void writeArray(const Array &array) {
      mpack_start_array(&writer, array.size());
      for (auto &value : array) write(value);
      mpack_finish_array(&writer);
    }

    void startArray(size_t length) { mpack_start_array(&writer, length); }
    void finishArray() { mpack_finish_array(&writer); }

    // Maps

    template<typename Key, typename Value>
    void writeMap(const etl::iflat_map<Key, Value> &map) {
      for (const auto &pair : map) writeKeyValue(pair);
    }

    void startMap(size_t length) { mpack_start_map(&writer, length); }
    void finishMap() { mpack_finish_map(&writer); }

    template<typename Key, typename Value>
    void writeKeyValue(const Util::Pair<Key, Value> &pair) {
      writeKeyValue(pair.first, pair.second);
    }
    template<typename Key, typename Value>
    void writeKeyValue(const Key &key, const Value &value) {
      write(key);
      write(value);
    }
    template<typename Value>
    void writeKeyValue(const char *string, uint32_t length, const Value &value) {
      writeString(string, length);
      write(value);
    }

    // Other

    template<typename Buffer>
    void writeSerializedBytes(const Buffer &buffer) {
      writeSerializedBytes(ByteBufferView(buffer));
    }
    void writeSerializedBytes(const ByteBufferView &buffer) {
      mpack_write_object_bytes(&writer, reinterpret_cast<const char *>(buffer.data()), buffer.size());
    }

    template<typename Class>
    bool writeClass(const Class &instance) { // return code is merely for whether the instance's write class succeeded, it's still recommended to check the mpack error with error()
      return instance.write(*this);
    }

  protected:
    ByteBuffer &buffer;
    const size_t startOffset;
    const size_t endOffset;

    const DocumentHeader &header;

    bool writeHeader() {
      return header.write(buffer);
    }
};

} } }