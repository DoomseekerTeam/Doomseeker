//------------------------------------------------------------------------------
// huffmanqt.cpp
//------------------------------------------------------------------------------
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA
//
//------------------------------------------------------------------------------
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "huffmanqt.h"

#include "huffman/huffman.h"

QByteArray HuffmanQt::decode(const QByteArray &in)
{
	return decode(in.constData(), in.size());
}

QByteArray HuffmanQt::decode(const char *in, int size)
{
	// Below comment is copypasted from Q-Zandronum's network.cpp:
	//
	// [BB] Vortex Cortex pointed us to the fact that the smallest huffman code is only 3 bits
	// and it turns into 8 bits when it's decompressed. Thus we need to allocate a buffer that
	// can hold the biggest possible size we may get after decompressing (aka Huffman decoding).

	int outSize = ((size * 8) / 3) + 1; // compressed/decompressed = 3/8
	auto out = new unsigned char[outSize];
	HUFFMAN_Decode(reinterpret_cast<const unsigned char *>(in), out, size, &outSize);
	QByteArray result(reinterpret_cast<char *>(out), outSize);
	delete [] out;
	return result;
}

QByteArray HuffmanQt::encode(const QByteArray &in)
{
	return encode(in.constData(), in.size());
}

QByteArray HuffmanQt::encode(const char *in, int size)
{
	int outSize = size + 1;
	auto out = new unsigned char[outSize];
	HUFFMAN_Encode(reinterpret_cast<const unsigned char *>(in), out, size, &outSize);
	QByteArray result(reinterpret_cast<char *>(out), outSize);
	delete [] out;
	return result;
}
