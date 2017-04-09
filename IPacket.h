#ifndef IPACKET_H
#define IPACKET_H

#include <vector>
#include <cstring>

class IPacket
{
	public:

		using byte = unsigned char;
		using size_type = unsigned int;

		virtual size_type vSize() const = 0;
		virtual const byte* vData() const = 0;

};

class BinaryPacket : public IPacket
{
	public:

		using ByteArray = std::vector<byte>;

		BinaryPacket(size_type size)
		{
			_data.resize(size, 0);
		}

		BinaryPacket(byte *data, size_type size)
		{
			_data.resize(size);
			memcpy(&_data[0], data, size);
		}

		size_type vSize() const override { return _data.size(); }
		const byte* vData() const override { return &_data[0]; }

	private:

		ByteArray _data;
};

#endif // IPACKET_H
