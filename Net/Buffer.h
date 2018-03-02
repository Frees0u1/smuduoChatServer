#ifndef SMUDUO_BUFFER_H
#define SMUDUO_BUFFER_H

#include <vector>
#include <string.h>
#include "../Base/Common.h"
#include <assert.h>
#include <algorithm>
#include <stdint.h>
#include <endian.h>

class Buffer : public smuduo::copyable {

public:
    static const size_t kCheapPrepend = 8; 
    static const size_t kInitialSize = 1024; //默认初始大小,1024字节

    explicit Buffer(size_t initialSize = kInitialSize)
        : buf_(kCheapPrepend + initialSize),
        readIdx_(kCheapPrepend),
        writeIdx_(kCheapPrepend)
    {

    }

    void swap(Buffer& rhs){
        buf_.swap(rhs.buf_);
        std::swap(readIdx_, rhs.readIdx_);
        std::swap(writeIdx_, rhs.writeIdx_);
    }

    size_t readableBytes() const {
        return writeIdx_ - readIdx_;
    }

    size_t writableBytes() const {
        return buf_.size() - writeIdx_;
    }

    size_t prependableBytes() const {
        return readIdx_;
    }

    void ensureWritableBytes(size_t len) {
        if(writableBytes() < len){
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    const char* peek() const { //peek即找到第一个可写的字符
        return begin() + readIdx_;
    }
    char* beginWrite() {
        return begin() + writeIdx_;
    }
    const char* beginWrite() const {
        return begin() + writeIdx_;
    }

    void retrieve(size_t len) {
        assert(len <= readableBytes());
        if (len < readableBytes()) {
            readIdx_ += len;
        }
        else{
            retrieveAll();
        }
    }

    void retrieveUntill(const char* end) { 
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }
    
    //
    //retrieveInt family
    //
    void retrieveInt64() {
        retrieve(sizeof(int64_t));
    }
    void retrieveInt32() {
        retrieve(sizeof(int32_t));
    }
    void retrieveInt16() {
        retrieve(sizeof(int16_t));
    }
    void retrieveInt8() {
        retrieve(sizeof(int8_t));
    }

    void retrieveAll() {
        readIdx_ = kCheapPrepend;
        writeIdx_ = kCheapPrepend;
    }

    std::string retrieveAsString(size_t len) {
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    std::string retrieveAllAsString(){
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAllAsString_withPrepend(){
        std::string result(begin(), prependableBytes() + readableBytes());
        retrieveAll();
        return result;
    }

    void append(const char *data, size_t len) { //在后面添上一段字节
        ensureWritableBytes(len);
        std::copy(data, data+len, beginWrite());
        hasWritten(len);
    }

    void append(const std::string& str) {
        append(str.data(), str.size());
    }

    void append(const void* data, size_t len) {
        append(static_cast<const char*>(data), len);
    }

    // Append int family
    // Append int using network endian
    // sequences in Buffer
    void appendInt64(int64_t x) {
        int64_t be64 = htobe64(x);
        append(&be64, sizeof be64);
    }
    void appendInt32(int32_t x) {
        int32_t be32 = htobe32(x);
        append(&be32, sizeof be32);
    }
    void appendInt16(int16_t x) {
        int16_t be16 = htobe16(x);
        append(&be16, sizeof be16);
    }

    void appendInt8(int8_t x) {
        append(&x, sizeof x);
    }

    // Readint family
    // Read ints(64/32/16/8) from network endian
    // Require: buf->readableBytes() >= sizeof(int**) (** is 64/32/16..)
    int64_t readInt64(){
        //assert(readableBytes() >= sizeof(int64_t));
        int64_t result = peekInt64();
        retrieveInt64();
        return result;
    }
    int32_t readInt32() {
        //assert(readableBytes() >= sizeof(int32_t));
        int32_t result = peekInt32();
        retrieveInt32();
        return result;
    }
    int16_t readInt16() {
        //assert(readableBytes() >= sizeof(int16_t));
        int16_t result = peekInt16();
        retrieveInt16();
        return result;
    }
    int8_t readInt8() {
       // assert(readableBytes() >= sizeof(int8_t));
        int16_t result = peekInt8();
        retrieveInt8();
        return result;
    }
    
    //
    //peek int family
    //
    int64_t peekInt64() const {
        assert(readableBytes() >= sizeof(int64_t));
        int64_t be64 = 0;
        ::memcpy(&be64, peek(), sizeof be64);
        return be64toh(be64);
    }
    int32_t peekInt32() const {
        assert(readableBytes() >= sizeof(int32_t));
        int32_t be32 = 0;
        ::memcpy(&be32, peek(), sizeof be32);
        return be32toh(be32);
    }
    int16_t peekInt16() const {
        assert(readableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16, peek(), sizeof be16);
        return be16toh(be16);
    }
    int8_t peekInt8() const { // 1 Byte = 8 bits
        assert(readableBytes() >= sizeof(int8_t));
        int8_t x = *peek();
        return x;
    }

    //
    // prependInt family
    // initily there is 8B in front, using network endian
    void prependInt64(int64_t x){
        int64_t be64 = htobe64(x);
        prepend(&be64, sizeof be64);
    }
    void prependInt32(int32_t x){
        int32_t be32 = htobe32(x);
        prepend(&be32, sizeof be32);
    }
    void prependInt16(int16_t x){
        int16_t be16 = htobe16(x);
        prepend(&be16, sizeof be16);
    }
    void prependInt8(int8_t x){
//        int64_t = htobe64(x);
        prepend(&x, sizeof x);
    }    

    void prepend(const void* data, size_t len) {
        assert(len <= prependableBytes());
        readIdx_ -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d+len, begin() + readIdx_);
    }

    void hasWritten(size_t len) {
        assert(len <= writableBytes());
        writeIdx_ += len;
    }


    void unwrite(size_t len) {
        assert(len <= readableBytes());
        writeIdx_ -= len;
    }

    
    //读取fd中的数据，并将其置入进Buffer中
    ssize_t readFd(int fd, int* savedErrno);

	
private:
	char* begin() {
		return &*buf_.begin();	
	}

	const char* begin() const {
		return &*buf_.begin();
	}
	
	void makeSpace(size_t len){
		//空间不足，申请空间
		if(writableBytes() + prependableBytes() < len + kCheapPrepend) {
			buf_.resize(writeIdx_ + len);	
		}
		else { //prepend太长，内部腾挪
			assert(kCheapPrepend < readIdx_);
			size_t readable = readableBytes();
			std::copy(begin() + readIdx_,
					  begin() + writeIdx_,
					  begin() + kCheapPrepend);
			readIdx_ = kCheapPrepend;
			writeIdx_ = readIdx_ + readable;
			assert(readable == readableBytes());
		}
	}

private:
    std::vector<char> buf_;
    int readIdx_;
    int writeIdx_;
};



#endif
