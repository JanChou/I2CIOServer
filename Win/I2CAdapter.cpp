/**
 * (c)2016 CDP Technologies AS
 */

#include "I2CAdapter.h"

using namespace std;

class I2CAdapter::Private
{
};

I2CAdapter::I2CAdapter()
  : d(new Private)
{
}

I2CAdapter::~I2CAdapter()
{
}

void I2CAdapter::Open(const string&)
{
}

void I2CAdapter::SetAddress(uint8_t)
{
}

vector<uint8_t> I2CAdapter::Read(uint8_t, vector<uint8_t>::size_type size)
{
  return vector<uint8_t>(size);
}

void I2CAdapter::Write(uint8_t, const vector<uint8_t>&)
{
}

void I2CAdapter::Close()
{
}
