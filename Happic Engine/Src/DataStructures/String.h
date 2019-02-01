#pragma once

#include "Types.h"
#include <iostream>

namespace Happic {

	class String
	{
	public:
		String(cstring characters);
		String(uint32 length);
		String(const String& string);
		String();
		~String();

		uint32 Length() const;

		cstring C_Str() const;

		int32 FindFirstOf(const String& str, uint32 offset = 0) const;
		int32 FindFirstNotOf(const String& str, uint32 offset = 0) const;
		int32 FindFirstCharacter(uint32 offset = 0) const;
		int32 FindFirstSpace(uint32 offset = 0) const;

		String RemoveAllOf(const String& str) const;

		String TrimBeginning() const;
		String TrimEnding() const;
		String Trim();

		String SubString(uint32 startIndex, uint32 endIndex) const;
		String SubString(uint32 startIndex) const;

		String Cut(uint32 start, uint32 end) const;

		bool BeginsWith(const String& string) const;

		bool IsEmpty() const;

		char operator[](uint32 index) const;
		char& operator[](uint32 index);

		bool operator==(const String& string) const;
		bool operator<(const String& string) const;

		String& operator=(const String& string);
		String& operator=(cstring characters);

		String operator+(const String& string) const;

		String& operator+=(const String& string);

		friend std::ostream& operator<<(std::ostream& out, const String& string);
	private:
		void Copy(cstring characters);
		void Copy(const String& string);
	private:
		char* m_characters;
		uint32 m_length;
	};

}
