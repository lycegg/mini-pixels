//
// Created by yuly on 05.04.23.
//

#include "vector/DecimalColumnVector.h"
#include "duckdb/common/types/decimal.hpp"
#include <algorithm>

/**
 * The decimal column vector with precision and scale.
 * The values of this column vector are the unscaled integer value
 * of the decimal. For example, the unscaled value of 3.14, which is
 * of the type decimal(3,2), is 314. While the precision and scale
 * of this decimal are 3 and 2, respectively.
 *
 * <p><b>Note: it only supports short decimals with max precision
 * and scale 18.</b></p>
 *
 * Created at: 05/03/2022
 * Author: hank
 */

DecimalColumnVector::DecimalColumnVector(int precision, int scale, bool encoding): ColumnVector(VectorizedRowBatch::DEFAULT_SIZE, encoding) {
    DecimalColumnVector(VectorizedRowBatch::DEFAULT_SIZE, precision, scale, encoding);
}

DecimalColumnVector::DecimalColumnVector(uint64_t len, int precision, int scale,
                                         bool encoding)
    : ColumnVector(len, encoding) {
    // decimal column vector has no encoding so we don't allocate memory to
    // this->vector
    this->vector = nullptr;
    this->precision = precision;
    this->scale = scale;

    using duckdb::Decimal;
    /*if (precision <= Decimal::MAX_WIDTH_INT16) {
        physical_type_ = PhysicalType::INT16;
        posix_memalign(reinterpret_cast<void **>(&vector), 32,
                       len * sizeof(int16_t));
        memoryUsage += (uint64_t)sizeof(int16_t) * len;
    } else if (precision <= Decimal::MAX_WIDTH_INT32) {
        physical_type_ = PhysicalType::INT32;
        posix_memalign(reinterpret_cast<void **>(&vector), 32,
                       len * sizeof(int32_t));
        memoryUsage += (uint64_t)sizeof(int32_t) * len;
    } else */if (precision <= Decimal::MAX_WIDTH_INT64) {
        physical_type_ = PhysicalType::INT64;
        posix_memalign(reinterpret_cast<void **>(&vector), 32,
                       len * sizeof(int64_t));
        memoryUsage += (uint64_t)sizeof(uint64_t) * len;
    } else if (precision <= Decimal::MAX_WIDTH_INT128) {
        throw std::runtime_error(
            "Decimal precision is bigger than the maximum supported width");
        physical_type_ = PhysicalType::INT128;
        memoryUsage += (uint64_t)sizeof(uint64_t) * len;
    } else {
        throw std::runtime_error(
            "Decimal precision is bigger than the maximum supported width");
    }
}

void DecimalColumnVector::close() {
    if (!closed) {
        ColumnVector::close();
        //if (physical_type_ == PhysicalType::INT16 ||
        //    physical_type_ == PhysicalType::INT32) {
        if(vector!=nullptr){
            //free(vector);
        }
        vector = nullptr;
    }
}

void DecimalColumnVector::print(int rowCount) {
//    throw InvalidArgumentException("not support print Decimalcolumnvector.");
    for(int i = 0; i < rowCount; i++) {
        std::cout<<vector[i]<<std::endl;
    }
}

DecimalColumnVector::~DecimalColumnVector() {
    if(!closed) {
        DecimalColumnVector::close();
    }
}

void * DecimalColumnVector::current() {
    if(vector == nullptr) {
        return nullptr;
    } else {
        return vector + readIndex;
    }
}

int DecimalColumnVector::getPrecision() {
	return precision;
}


int DecimalColumnVector::getScale() {
	return scale;
}
long getval(std::string&val,int prec=6){
    int dian=0;
    while(dian<val.size()&&val[dian]!='.')dian++;
    long ans=stol(val.substr(0,dian));
    dian++;
    while(dian<val.size()){
        ans=ans*10+val[dian]-'0';
        prec--;
        dian++;
    }
    while(prec>0){
        ans=ans*10;
        prec--;
    }
    return ans;
}

void DecimalColumnVector::add(std::string &value) {
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    if (value == "true") {
        add(1);
    } else if (value == "false") {
        add(0);
    } else {
        add(getval(value,precision));
    }
}

void DecimalColumnVector::add(bool value) {
    add(value ? 1 : 0);
}

void DecimalColumnVector::add(int64_t value) {
    if (writeIndex >= length) {
        ensureSize(writeIndex * 2, true);
    }
    int index = writeIndex++;
    vector[index]=value;
    //printf("added%lld\n",value);
    isNull[index] = false;
}

void DecimalColumnVector::add(int value) {
    if (writeIndex >= length) {
        ensureSize(writeIndex * 2, true);
    }
    int index = writeIndex++;
    vector[index]=value;
    //printf("added%d\n",value);
    isNull[index] = false;
}

void DecimalColumnVector::ensureSize(uint64_t size, bool preserveData) {
    ColumnVector::ensureSize(size, preserveData);
    if (length < size) {
        long *oldVector = vector;
        posix_memalign(reinterpret_cast<void **>(&vector), 32,
                        size * sizeof(int64_t));
        if (preserveData) {
            std::copy(oldVector, oldVector + length, vector);
        }
        delete[] oldVector;
        memoryUsage += (long) sizeof(long) * (size - length);
        resize(size);
    }
}