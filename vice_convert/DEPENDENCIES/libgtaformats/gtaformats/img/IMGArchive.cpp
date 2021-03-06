/*
	Copyright 2010-2013 David "Alemarius Nexus" Lerch

	This file is part of gtaformats.

	gtaformats is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	gtaformats is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with gtaformats.  If not, see <http://www.gnu.org/licenses/>.

	Additional permissions are granted, which are listed in the file
	GPLADDITIONS.
 */

#include "IMGArchive.h"
#include "IMGException.h"
#include <string>
#include <cstring>
#include <vector>
#include "../util/stream/RangedStream.h"
#include "../util/File.h"
#include <algorithm>
#include <cmath>
#include <utility>
#include <iterator>
#include "../util/stream/streamutil.h"
#include "../util/stream/StreamReader.h"
#include "../util/stream/EndianSwappingStreamReader.h"
#include "../util/stream/StreamWriter.h"
#include "../util/stream/EndianSwappingStreamWriter.h"

using std::iostream;
using std::string;
using std::vector;
using std::streamoff;
using std::sort;
using std::copy;
using std::min;
using std::pair;
using std::distance;

#define IMG_BUFFER_SIZE 4096

// Maximum number of blocks to be kept in memory
#define IMG_MAX_RAM_BLOCKS 25000 // ~ 51.2MB


char _DummyBuffer[8192] = {0};

bool _EntrySortComparator(const IMGEntry* e1, const IMGEntry* e2)
{
	return e1->offset < e2->offset;
}





IMGArchive::IMGArchive(istream* stream, int mode)
		: imgStream(stream), dirStream(stream), mode(mode)
{
	readHeader();
}


IMGArchive::IMGArchive(const File& file, int mode)
		: mode(mode)
{
	const FilePath* path = file.getPath();
	FileContentType type = file.guessContentType();

	if (type == CONTENT_TYPE_DIR) {
		char* imgFilePath = new char[path->toString().length() + 1];
		int basePathLen = path->toString().length() - 3;
		strncpy(imgFilePath, path->toString().get(), basePathLen);
		strcpy(imgFilePath+basePathLen, "img");
		File imgFile(imgFilePath);
		delete[] imgFilePath;

		if ((mode & ReadOnly)  !=  0) {
			dirStream = file.openInputStream(istream::binary);
			imgStream = imgFile.openInputStream(istream::binary);
		} else {
			dirStream = file.openInputOutputStream(iostream::binary);
			imgStream = file.openInputOutputStream(iostream::binary);
		}

		readHeader();
	} else if (type == CONTENT_TYPE_IMG) {
		if ((mode & ReadOnly)  !=  0) {
			imgStream = file.openInputStream(istream::binary);
		} else {
			imgStream = file.openInputOutputStream(iostream::binary);
		}

		if (guessIMGVersion(imgStream, true) == VER2) {
			dirStream = imgStream;
			readHeader();
		} else {
			char* dirFilePath = new char[path->toString().length() + 1];
			int basePathLen = path->toString().length() - 3;
			strncpy(dirFilePath, path->toString().get(), basePathLen);
			strcpy(dirFilePath+basePathLen, "dir");
			File dirFile(dirFilePath);
			delete[] dirFilePath;

			if ((mode & ReadOnly)  !=  0) {
				dirStream = dirFile.openInputStream(istream::binary);
			} else {
				dirStream = dirFile.openInputOutputStream(istream::binary);
			}

			readHeader();
		}
	} else {
		throw IMGException("File name is neither an IMG nor a DIR file", __FILE__, __LINE__);
	}
}


IMGArchive::IMGArchive(istream* dirStream, istream* imgStream, int mode)
		: imgStream(imgStream), dirStream(dirStream), mode(mode)
{
	readHeader();
}


IMGArchive::IMGArchive(const File& dirFile, const File& imgFile, int mode)
		: mode(mode)
{
	if ((mode & ReadOnly)  !=  0) {
		imgStream = imgFile.openInputStream(istream::binary);
		dirStream = dirFile.openInputStream(istream::binary);
	} else {
		imgStream = imgFile.openInputOutputStream(iostream::binary);
		dirStream = dirFile.openInputOutputStream(iostream::binary);
	}

	readHeader();
}


IMGArchive::~IMGArchive()
{
	sync();

	EntryIterator it;
	for (it = entries.begin() ; it != entries.end() ; it++) {
		delete *it;
	}

	for (EntryMap::iterator it = entryMap.begin() ; it != entryMap.end() ; it++) {
//		delete[] (it->first);
	}

	if ((mode & KeepStreamsOpen)  ==  0) {
		if (imgStream != dirStream) {
			delete imgStream;
			delete dirStream;
		} else {
			delete imgStream;
		}
	}
}


IMGArchive::IMGVersion IMGArchive::guessIMGVersion(istream* stream, bool returnToStart)
{
	char fourCC[4];
	stream->read(fourCC, 4);

	if (stream->eof()) {
		return VER1;
	}

	if (returnToStart) {
		stream->seekg(-4, istream::cur);
	}

	if (fourCC[0] == 'V'  &&  fourCC[1] == 'E'  &&  fourCC[2] == 'R'  &&  fourCC[3] == '2') {
		return VER2;
	} else {
		return VER1;
	}
}


IMGArchive::IMGVersion IMGArchive::guessIMGVersion(const File& file)
{
	//FileInputStream stream(file, STREAM_BINARY);
	istream* stream = file.openInputStream(istream::binary);
	IMGVersion ver = guessIMGVersion(stream, false);
	delete stream;
	return ver;
}


IMGArchive* IMGArchive::createArchive(iostream* imgStream, int mode)
{
#ifdef GTAFORMATS_LITTLE_ENDIAN
	StreamWriter writer(imgStream);
#else
	EndianSwappingStreamWriter writer(imgStream);
#endif

	uint32_t numEntries = 0;
	imgStream->write("VER2", 4);
	writer.writeU32(numEntries);
	imgStream->flush();
	imgStream->seekg(-8, iostream::cur);

	return new IMGArchive(imgStream, mode);
}


IMGArchive* IMGArchive::createArchive(iostream* dirStream, iostream* imgStream, int mode)
{
	// Nothing to be done. VER1 archives don't even have a header, so we don't have to write anything.
	return new IMGArchive(dirStream, imgStream, mode);
}


IMGArchive* IMGArchive::createArchive(const File& file, IMGVersion version, int mode)
{
	if (version == VER1) {
		if (file.guessContentType() == CONTENT_TYPE_DIR) {
			const char* dirPath = file.getPath()->toString().get();
			char* imgPath = new char[strlen(dirPath)+1];
			int baseLen = strlen(dirPath) - file.getPath()->getExtension().length();
			strncpy(imgPath, dirPath, baseLen);
			strcpy(imgPath+baseLen, "img");
			File imgFile(imgPath);
			delete[] imgPath;
			return createArchive(file.openInputOutputStream(iostream::binary),
					imgFile.openInputOutputStream(iostream::binary), mode);
		} else {
			const char* imgPath = file.getPath()->toString().get();
			char* dirPath = new char[strlen(imgPath)+1];
			int baseLen = strlen(imgPath) - file.getPath()->getExtension().length();
			strncpy(dirPath, imgPath, baseLen);
			strcpy(dirPath+baseLen, "dir");
			File dirFile(dirPath);
			delete[] dirPath;
			return createArchive(dirFile.openInputOutputStream(iostream::binary),
					file.openInputOutputStream(iostream::binary), mode);
		}
	} else {
		return createArchive(file.openInputOutputStream(iostream::binary), mode);
	}
}


IMGArchive* IMGArchive::createArchive(const File& dirFile, const File& imgFile, int mode)
{
	return createArchive(dirFile.openInputOutputStream(iostream::binary),
			imgFile.openInputOutputStream(iostream::binary), mode);
}


istream* IMGArchive::gotoEntry(ConstEntryIterator it, bool autoCloseStream)
{
	IMGEntry* entry = *it;
	long long start = entry->offset*IMG_BLOCK_SIZE;

	if ((mode & ReadOnly)  !=  0) {
		imgStream->seekg(start - imgStream->tellg(), istream::cur);
		RangedStream<istream>* rstream = new RangedStream<istream>(imgStream,
				IMG_BLOCKS2BYTES(entry->size), autoCloseStream);
		return rstream;
	} else {
		iostream* outImgStream = static_cast<iostream*>(imgStream);
		imgStream->seekg(start - imgStream->tellg(), istream::cur);
		outImgStream->seekp(start - outImgStream->tellp(), fstream::cur);
		RangedStream<iostream>* rstream = new RangedStream<iostream>(outImgStream,
				IMG_BLOCKS2BYTES(entry->size), autoCloseStream);
		return rstream;
	}
}


istream* IMGArchive::gotoEntry(const char* name, bool autoCloseStream)
{
	ConstEntryIterator it = getEntryByName(name);

	if (it == entries.end()) {
		return NULL;
	}

	return gotoEntry(it, autoCloseStream);
}


IMGArchive::ConstEntryIterator IMGArchive::getEntryByName(const char* name) const
{
	char* lname = new char[strlen(name)+1];
	strtolower(lname, name);
	EntryMap::const_iterator it = entryMap.find(lname);
	delete[] lname;

	if (it == entryMap.end()) {
		return entries.end();
	}

	return it->second;
}


IMGArchive::EntryIterator IMGArchive::getEntryByName(const char* name)
{
	char* lname = new char[strlen(name)+1];
	strtolower(lname, name);
	EntryMap::iterator it = entryMap.find(lname);
	delete[] lname;

	if (it == entryMap.end()) {
		return entries.end();
	}

	return it->second;
}


void IMGArchive::readHeader()
{
	istream* stream = dirStream;

#ifdef GTAFORMATS_LITTLE_ENDIAN
	StreamReader reader(stream);
#else
	EndianSwappingStreamReader reader(stream);
#endif

	int32_t firstBytes;

	stream->read((char*) &firstBytes, 4);
	streamoff gcount = stream->gcount();

	if (stream->eof()) {
		if (gcount == 0) {
			// This is an empty file: VER1 DIR files can be completely empty, so we assume this one is
			// such a file.

			version = VER1;
			return;
		} else {
			throw IMGException("Premature end of file", __FILE__, __LINE__);
		}
	}

	char* fourCC = (char*) &firstBytes;

	vector<IMGEntry*> entryVector;

	if (fourCC[0] == 'V'  &&  fourCC[1] == 'E'  &&  fourCC[2] == 'R'  &&  fourCC[3] == '2') {
		version = VER2;
		int32_t numEntries = reader.readU32();

		for (int32_t i = 0 ; i < numEntries ; i++) {
			IMGEntry* entry = new IMGEntry;
			stream->read((char*) entry, sizeof(IMGEntry));

#ifndef GTAFORMATS_LITTLE_ENDIAN
			entry->offset = SwapEndianness32(entry->offset);
			entry->size = SwapEndianness32(entry->size);
#endif

			entryVector.push_back(entry);
		}
	} else {
		version = VER1;

		// In VER1 'firstBytes' is actually part of the first entry, so we have to handle it manually.
		IMGEntry* firstEntry = new IMGEntry;

		firstEntry->offset = ToLittleEndian32(firstBytes);

		stream->read(((char*) firstEntry)+4, sizeof(IMGEntry)-4);

#ifndef GTAFORMATS_LITTLE_ENDIAN
		firstEntry->size = SwapEndianness32(firstEntry->size);
#endif

		entryVector.push_back(firstEntry);

		while (!stream->eof()) {
			IMGEntry* entry = new IMGEntry;
			stream->read((char*) entry, sizeof(IMGEntry));

#ifndef GTAFORMATS_LITTLE_ENDIAN
			entry->offset = SwapEndianness32(entry->offset);
			entry->size = SwapEndianness32(entry->size);
#endif

			streamoff lrc = stream->gcount();

			if (lrc != sizeof(IMGEntry)) {
				if (lrc == 0) {
					delete entry;
					break;
				} else {
					char errmsg[128];
					sprintf(errmsg, "ERROR: Input isn't divided into %u-byte blocks. Is this really a "
							"VER1 DIR file?", (unsigned int) sizeof(IMGEntry));
					throw IMGException(errmsg, __FILE__, __LINE__);
				}
			}

			entryVector.push_back(entry);
		}
	}

	// TODO: All IMG files I've seen so far are already sorted by offset, but I'm not sure if this is
	// always true, so we don't take any risks here...
	sort(entryVector.begin(), entryVector.end(), _EntrySortComparator);
	entries = EntryList(entryVector.begin(), entryVector.end());

	for (EntryIterator it = entries.begin() ; it != entries.end() ; it++) {
		IMGEntry* entry = *it;
		char* lname = new char[24];
		strtolower(lname, entry->name);
		string slname = lname;
		delete[] lname;
		entryMap.insert(pair<string, EntryIterator>(slname, it));
	}

	if (entries.size() > 0) {
		headerReservedSpace = (*entries.begin())->offset;
	} else {
		headerReservedSpace = 1;
	}
}


int32_t IMGArchive::getHeaderReservedSize() const
{
	if (version == VER1) {
		// DIR files don't have something as a reserved size.
		return -1;
	} else {
		if (entries.size() > 0) {
			return (*entries.begin())->offset;
		} else {
			return -1;
		}
	}
}


void IMGArchive::forceMoveEntries(EntryIterator pos, EntryIterator moveBegin, EntryIterator moveEnd,
		int32_t newOffset)
{
	if (moveBegin == moveEnd)
		return;

	EntryIterator lastToBeMoved = moveEnd;
	lastToBeMoved--;
	IMGEntry* moveBeginEntry = *moveBegin;
	IMGEntry* lastMoveEntry = *lastToBeMoved;

	iostream* outImgStream = static_cast<iostream*>(imgStream);

	// Move the contents of all entries between moveBegin and moveEnd to newOffset
	int32_t moveCount = (lastMoveEntry->offset + lastMoveEntry->size) - moveBeginEntry->offset;

	imgStream->seekg(IMG_BLOCKS2BYTES(moveBeginEntry->offset), istream::beg);

	StreamMove(outImgStream, IMG_BLOCKS2BYTES(moveCount), IMG_BLOCKS2BYTES(newOffset));

	// The offset by which all moved entries have been moved.
	int32_t relativeOffset = newOffset - moveBeginEntry->offset;

	size_t numMoved = 0;

	// Update the offsets in the header
	for (EntryIterator it = moveBegin ; it != moveEnd ; it++) {
		IMGEntry* entry = *it;
		entry->offset += relativeOffset;
		numMoved++;
	}

	// And move them in the 'entries' list
	// Note that after calling entries.insert(), all iterators still point to the same elements they pointed
	// to before. However, when moveEnd points to entries.end(), it should afterwards still point to the
	// end of the moved-range, and not the end of the whole list, so we let oldMoveEnd point to the last moved
	// entry, and afterwards increment it again to point to the old location of moveEnd.
	EntryIterator oldMoveEnd = moveEnd;
	oldMoveEnd--;
	entries.insert(pos, moveBegin, moveEnd);
	oldMoveEnd++;
	entries.erase(moveBegin, oldMoveEnd);

	// Update the entry map. Note that the only valid iterator we have here is 'pos'.
	EntryIterator it = pos;
	for (size_t i = 0 ; i < numMoved ; i++) {
		it--;
		IMGEntry* entry = *it;

		char lname[24];
		strtolower(lname, entry->name);
		entryMap[lname] = it;
	}
}


void IMGArchive::moveEntries(EntryIterator pos, EntryIterator moveBegin, EntryIterator moveEnd,
		MoveMode mmode)
{
	if (moveBegin == moveEnd)
		return;

	// Check if pos is between moveBegin and moveEnd. In this case, there's nothing to be moved.
	for (EntryIterator it = moveBegin ; it != moveEnd ; it++) {
		if (it == pos)
			return;
	}
	if (pos == moveEnd)
		return;

	// Now we know that pos is either before or after the move range.

	if (mmode == MoveToEnd) {
		// We move enough entries following the insertion position to the end of the archive.

		int32_t newStartOffset; // Offset of the insertion position

		if (pos == entries.begin()) {
			newStartOffset = (*pos)->offset;
		} else {
			// There might be some holes before endMoveBegin. If so, we can fill them with this move
			// operation.
			EntryIterator beforePos = pos;
			beforePos--;
			newStartOffset = (*beforePos)->offset + (*beforePos)->size;
		}

		if (pos != entries.end()) {
			// If the entries are to be inserted somewhere in the middle of the archive, we first have to
			// make place for them...

			int32_t moveBeginOffs = (*moveBegin)->offset;

			EntryIterator last = moveEnd;
			last--;
			int32_t moveEndOffs = (*last)->offset + (*last)->size;

			int32_t moveSize = moveEndOffs - moveBeginOffs; // Number of blocks to be moved

			EntryIterator endMoveBegin = pos; // First entry to be moved to the end
			EntryIterator endMoveEnd; // One PAST the last entry to be moved to the end
			int32_t endMoveBeginOffset = newStartOffset;

			for (	endMoveEnd = pos
					;
					endMoveEnd != entries.end() // Stop if the archive end is reached
					&&  endMoveEnd != moveBegin // Stop if the move range is reached
					&&  (*endMoveEnd)->offset - endMoveBeginOffset < moveSize
							// Stop if enough space is available between endMoveBegin and endMoveEnd
					;
					endMoveEnd++
			) {}

			int32_t endOffset = getDataEndOffset();

			// We might need to move the entries even futher than the current archive end
			if (endMoveEnd == entries.end()) {
				endOffset += moveSize - (endOffset - endMoveBeginOffset);
			}

			pos = endMoveEnd;

			// Now move the entries to the end (and maybe futher)
			forceMoveEntries(entries.end(), endMoveBegin, endMoveEnd, endOffset);
		}

		// Now we can be sure that there is enough space left after the insertion position.

		if (pos != moveBegin) {
			// And finally, perform the actual requested move
			forceMoveEntries(pos, moveBegin, moveEnd, newStartOffset);
		}

		expandSize();
	} else {
		// We create two consecutive ranges between moveBegin, moveEnd and pos, and swap them.

		bool moveForward = false;

		// Determine whether pos is before or after the move range (unfortunately, with bidi iterators,
		// there's no better way to do this)
		for (EntryIterator it = entries.begin() ;; it++) {
			if (it == pos) {
				moveForward = false;
				break;
			} else if (it == moveBegin) {
				moveForward = true;
				break;
			}
		}

		if (moveForward) {
			swapConsecutive(moveBegin, moveEnd, pos);
		} else {
			swapConsecutive(pos, moveBegin, moveEnd);
		}
	}
}


bool IMGArchive::reserveHeaderSpace(int32_t numHeaders)
{
	if ((mode & ReadWrite)  ==  0) {
		throw IMGException("Attempt to modify a read-only IMG archive!", __FILE__, __LINE__);
	}

	int64_t bytesToReserve = numHeaders * sizeof(IMGEntry);

	if (version == VER2) {
		// Consider the IMG header for VER2
		bytesToReserve += 8;
	}

	int64_t reservedSize = getHeaderReservedSize();

	headerReservedSpace = ceil((float) bytesToReserve / IMG_BLOCK_SIZE);

	if (reservedSize == -1) {
		// VER1 or empty VER2 archive. In both cases, there's nothing to reserve.
		return true;
	}

	reservedSize *= IMG_BLOCK_SIZE; // reservedSize shall be in bytes

	if (bytesToReserve <= reservedSize) {
		return true;
	}

	// This has to be a VER2 archive, because reservedSize would be -1 for VER1 archives.

	EntryIterator moveBegin = entries.begin();
	EntryIterator moveEnd; // One PAST the last entry to be moved

	// Find out how many entries we have to move
	for (moveEnd = entries.begin() ; moveEnd != entries.end() ; moveEnd++) {
		IMGEntry* entry = *moveEnd;
		if (entry->offset*IMG_BLOCK_SIZE >= bytesToReserve) {
			break;
		}
	}

	moveEntries(entries.end(), moveBegin, moveEnd);

	return true;
}


void IMGArchive::rewriteHeaderSection()
{
	if ((mode & ReadWrite)  ==  0) {
		throw IMGException("Attempt to modify a read-only IMG archive!", __FILE__, __LINE__);
	}

	iostream* outImgStream = static_cast<iostream*>(imgStream);

#ifdef GTAFORMATS_LITTLE_ENDIAN
	StreamWriter writer(outImgStream);
#else
	EndianSwappingStreamWriter writer(outImgStream);
#endif

	if (version == VER1) {
		outImgStream->seekp(0 - outImgStream->tellp(), ostream::cur);
	} else {
		outImgStream->seekp(4 - outImgStream->tellp(), ostream::cur);
		int32_t numEntries = entries.size();
		writer.write32(numEntries);
	}

	// Write the header section
	EntryIterator it;
	for (it = entries.begin() ; it != entries.end() ; it++) {
#ifdef GTAFORMATS_LITTLE_ENDIAN
		outImgStream->write((char*) *it, sizeof(IMGEntry));
#else
		writer.writeArrayCopyU32((uint32_t*) *it, 2);
		outImgStream->write(((char*) *it) + 8, sizeof(IMGEntry)-8);
#endif
	}
}


bool IMGArchive::addEntries(IMGEntry** newEntries, int32_t num)
{
	if ((mode & ReadWrite)  ==  0) {
		throw IMGException("Attempt to modify a read-only IMG archive!", __FILE__, __LINE__);
	}

	int32_t left = getReservedHeadersLeft();
	if (left != -1  &&  left < num) {
		return false;
	}

	int32_t offset = getDataEndOffset();

	for (int32_t i = 0 ; i < num ; i++) {
		IMGEntry* entry = newEntries[i];
		entry->offset = offset;
		offset += entry->size;
		entries.push_back(entry);

		if ((mode & DisableNameZeroFill) == 0) {
			size_t len = strlen(entry->name);
			memset(entry->name + len + 1, 0, 23 - len);
		}

		char* lname = new char[24];
		strtolower(lname, entry->name);
		entryMap[lname] = --entries.end();
	}

	expandSize();

	return true;
}


IMGArchive::EntryIterator IMGArchive::addEntry(const char* name, int32_t size)
{
	if ((mode & ReadWrite)  ==  0) {
		throw IMGException("Attempt to modify a read-only IMG archive!", __FILE__, __LINE__);
	}
	if (strlen(name) > 23) {
		throw IMGException("Maximum length of 23 characters for IMG entry names exceeded!",
				__FILE__, __LINE__);
	}

	IMGEntry* entry = new IMGEntry;
	size_t nameLen = strlen(name);
	//strcpy(entry->name, name);
	memcpy(entry->name, name, nameLen+1);
	entry->size = size;
	bool result = addEntries(&entry, 1);

	if (!result) {
		delete entry;
		return entries.end();
	}

	return getEntryByName(name);
}


void IMGArchive::resizeEntry(EntryIterator it, int32_t size)
{
	if ((mode & ReadWrite)  ==  0) {
		throw IMGException("Attempt to modify a read-only IMG archive!", __FILE__, __LINE__);
	}

	IMGEntry* entry = *it;

	EntryIterator next = it;
	next++;

	if (next == entries.end()) {
		// Nothing special has to be done to resize the last entry
		entry->size = size;
		expandSize();
		return;
	}

	IMGEntry* nextEntry = *next;

	if (nextEntry->offset-entry->offset < size) {
		// Too few space left, so we have to move the entry to the end
		moveEntries(entries.end(), it, next);
		entry->size = size;

		expandSize();
	} else {
		// Enough space left after the entry, so there's nothing special to be done
		entry->size = size;
	}
}


void IMGArchive::removeEntry(EntryIterator it)
{
	if ((mode & ReadWrite)  ==  0) {
		throw IMGException("Attempt to modify a read-only IMG archive!", __FILE__, __LINE__);
	}

	char lname[24];
	strtolower(lname, (*it)->name);
	EntryMap::iterator mit = entryMap.find(lname);
	/*char* key = mit->first;*/
	entryMap.erase(mit);
	/*delete[] key;*/

	delete *it;
	entries.erase(it);
}


void IMGArchive::renameEntry(EntryIterator it, const char* name)
{
	if ((mode & ReadWrite)  ==  0) {
		throw IMGException("Attempt to modify a read-only IMG archive!", __FILE__, __LINE__);
	}
	if (strlen(name) > 23) {
		throw IMGException("Maximum length of 23 characters for IMG entry names exceeded!",
				__FILE__, __LINE__);
	}

	IMGEntry* entry = *it;

	char lnameOld[24];
	strtolower(lnameOld, entry->name);
	EntryMap::iterator mit = entryMap.find(lnameOld);
	/*char* key = mit->first;*/
	entryMap.erase(mit);
	/*delete[] key;*/

	strcpy(entry->name, name);

	if ((mode & DisableNameZeroFill) == 0) {
		size_t len = strlen(entry->name);
		memset(entry->name + len + 1, 0, 23 - len);
	}

	char* lname = new char[24];
	strtolower(lname, name);
	entryMap.insert(pair<char*, EntryIterator>(lname, it));
}


void IMGArchive::expandSize(int32_t size)
{
	// Internal method, so no need to check for ReadWrite
	iostream* outImgStream = static_cast<iostream*>(imgStream);
	outImgStream->seekp(0, iostream::end);
	streamoff curSize = outImgStream->tellp();
	int bytesToAdd = IMG_BLOCKS2BYTES(size) - curSize;

	while (bytesToAdd > 0) {
		int numBytes = min(bytesToAdd, (int) sizeof(_DummyBuffer));
		outImgStream->write(_DummyBuffer, numBytes);
		bytesToAdd -= numBytes;
	}
}


void IMGArchive::expandSize()
{
	// Internal method, so no need to check for ReadWrite
	if (entries.size() > 0) {
		EntryIterator last = --entries.end();
		IMGEntry* entry = *last;
		expandSize(entry->offset + entry->size);
	}
}


int32_t IMGArchive::pack()
{
	if ((mode & ReadWrite)  ==  0) {
		throw IMGException("Attempt to modify a read-only IMG archive!", __FILE__, __LINE__);
	}

	// First, we create a copy of this IMGArchive
	File copyFile = File::createTemporaryFile();
	imgStream->seekg(0, istream::beg);
	copyFile.copyFrom(imgStream);

	// And a copy of it's entries
	IMGEntry** oldEntries = new IMGEntry*[entries.size()];
	EntryIterator it;
	int i = 0;
	for (it = entries.begin() ; it != entries.end() ; it++, i++) {
		oldEntries[i] = new IMGEntry;
		memcpy(oldEntries[i], *it, sizeof(IMGEntry));
	}

	// We calculate the minimum size needed for the header section
	int32_t headerSize = ceil((float) (entries.size() * sizeof(IMGEntry)) / IMG_BLOCK_SIZE);

	// Then we rebuild the header section
	int32_t offset = headerSize;
	for (it = entries.begin() ; it != entries.end() ; it++) {
		IMGEntry* entry = *it;
		entry->offset = offset;
		offset += entry->size;
	}

	// And now we reassign the content to each entry, reading it from the copy file.
	iostream* outImgStream = static_cast<iostream*>(imgStream);
	outImgStream->seekp(IMG_BLOCKS2BYTES(headerSize), ostream::beg);
	istream* oldImgStream = copyFile.openInputOutputStream(istream::binary);
	for (i = 0, it = entries.begin() ; it != entries.end() ; it++, i++) {
		// We don't have to reposition outImgStream, because the new entries are tightly packed.
		IMGEntry* oldEntry = oldEntries[i];
		oldImgStream->seekg(IMG_BLOCKS2BYTES(oldEntry->offset), istream::beg);
		char buf[4096];
		int bytesToRead = (int) IMG_BLOCKS2BYTES(oldEntry->size);

		while (bytesToRead != 0) {
			int numBytes = min(bytesToRead, (int) sizeof(buf));
			oldImgStream->read(buf, numBytes);
			outImgStream->write(buf, numBytes);
			bytesToRead -= numBytes;
		}
	}
	delete oldImgStream;

	// The archive is now successfully packed. Clean up
	delete[] oldEntries;
	copyFile.remove();

	return getDataEndOffset();
}


void IMGArchive::swapConsecutive(EntryIterator r1Begin, EntryIterator r1End, EntryIterator r2End)
{
	if (r1Begin == r1End  ||  r1End == r2End)
		return;

	iostream* outImgStream = static_cast<iostream*>(imgStream);

	EntryIterator r1Last = r1End;
	EntryIterator r2Last = r2End;
	r1Last--;
	r2Last--;

	int32_t r1Size = (*r1Last)->offset+(*r1Last)->size - (*r1Begin)->offset;
	int32_t r2Size = (*r2Last)->offset+(*r2Last)->size - (*r1End)->offset;

	int32_t backupSize; // In IMG blocks

	// For swapping, we need to store a copy of the contents of one range. We choose the smaller one.
	if (r1Size < r2Size) {
		imgStream->seekg((*r1Begin)->offset*IMG_BLOCK_SIZE - imgStream->tellg(), istream::cur);
		backupSize = r1Size;
	} else {
		imgStream->seekg((*r1End)->offset*IMG_BLOCK_SIZE - imgStream->tellg(), istream::cur);
		backupSize = r2Size;
	}

	uint8_t* backup;
	File* tmpFile;

	if (backupSize <= IMG_MAX_RAM_BLOCKS) {
		// Read the backup completely into memory.

		backup = new uint8_t[backupSize*IMG_BLOCK_SIZE];
		imgStream->read((char*) backup, backupSize*IMG_BLOCK_SIZE);
	} else {
		// Write the backup to a temporary file.

		tmpFile = new File(File::createTemporaryFile());
		ostream* out = tmpFile->openOutputStream(ostream::binary);

		char buf[IMG_BLOCK_SIZE];

		for (int32_t i = 0 ; i < backupSize ; i++) {
			imgStream->read(buf, sizeof(buf));
			out->write(buf, sizeof(buf));

			if (out->fail()) {
				delete out;
				tmpFile->remove();
				delete tmpFile;

				IOException ex("IO error writing backup for IMGArchive::swapConsecutive()! Maybe you don't "
						"have enough space left on the hard disk.", __FILE__, __LINE__);
				throw ex;
			}
		}

		delete out;
	}

	if (r1Size < r2Size) {
		// Now move the contents of range 2 to the beginning of range 1
		forceMoveEntries(r1Begin, r1End, r2End, (*r1Begin)->offset);

		r1End = r1Last;
		r1End++;

		// Update the offsets of all entries in range 1
		for (EntryIterator it = r1Begin ; it != r1End ; it++) {
			IMGEntry* entry = *it;
			entry->offset += r2Size;
		}

		// And rewrite the contents of range 1 from the backup
		outImgStream->seekp((*r1Begin)->offset*IMG_BLOCK_SIZE - outImgStream->tellp(), iostream::cur);
	} else {
		// Range 2 is smaller
		int32_t oldR1BeginOffset = (*r1Begin)->offset;

		// Update the offsets of all entries in range 2
		for (EntryIterator it = r1End ; it != r2End ; it++) {
			IMGEntry* entry = *it;
			entry->offset -= r1Size;
		}

		// Now move the contents of range 1 r2Size blocks further
		forceMoveEntries(r2End, r1Begin, r1End, (*r2Last)->offset + (*r2Last)->size);

		// And rewrite the contents of range 2 from the backup
		outImgStream->seekp(oldR1BeginOffset*IMG_BLOCK_SIZE - outImgStream->tellp(), iostream::cur);
	}

	if (backupSize <= IMG_MAX_RAM_BLOCKS) {
		// Restore the backup from memory

		outImgStream->write((char*) backup, backupSize*IMG_BLOCK_SIZE);
		delete[] backup;
	} else {
		// Restore the backup from the temporary file

		istream* in = tmpFile->openInputStream(istream::binary);

		char buf[IMG_BLOCK_SIZE];

		for (int32_t i = 0 ; i < backupSize ; i++) {
			in->read(buf, sizeof(buf));
			outImgStream->write(buf, sizeof(buf));
		}

		delete in;
		tmpFile->remove();
		delete tmpFile;
	}
}


void IMGArchive::zeroFill(int32_t size)
{
	// Internal method, so no need to check for ReadWrite
	iostream* outImgStream = static_cast<iostream*>(imgStream);
	int64_t bytesLeft = IMG_BLOCKS2BYTES(size);

	while (bytesLeft > 0) {
		int numBytes = min(bytesLeft, (int64_t) sizeof(_DummyBuffer));
		outImgStream->write(_DummyBuffer, numBytes);
		bytesLeft -= numBytes;
	}
}


