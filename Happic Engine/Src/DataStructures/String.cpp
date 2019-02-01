#include "String.h"
#include <cstring>

namespace Happic {

	String::String(cstring characters)
	{
		Copy(characters);
	}

	String::String(uint32 length) :
		m_length(length)
	{
		m_characters = new char[m_length + 1];
		m_characters[m_length] = '\0';
	}

	String::String(const String & string)
	{
		Copy(string);
	}

	String::String()
	{
		m_characters = NULL;
		m_length = 0;
	}

	String::~String()
	{
		if(m_length > 0)
			delete[] m_characters;
	}

	uint32 String::Length() const
	{
		return m_length;
	}

	cstring String::C_Str() const
	{
		return m_characters;
	}

	int32 String::FindFirstOf(const String & str, uint32 offset) const
	{
		uint32 compareIndex = 0;
		for (uint32 i = offset; i < m_length; i++)
		{
			if (m_characters[i] == str[compareIndex])
				compareIndex++;

			if (compareIndex == str.m_length)
				return i - compareIndex + 1;
		}
		return -1;
	}

	int32 String::FindFirstNotOf(const String & str, uint32 offset) const
	{
		uint32 compareIndex = 0;
		for (uint32 i = offset; i < m_length; i++)
		{
			if (m_characters[i] != str[compareIndex])
				compareIndex++;

			if (compareIndex == str.m_length)
				return i - compareIndex + 1;
		}
		return -1;
	}

	int32 String::FindFirstCharacter(uint32 offset) const
	{
		for (uint32 i = offset; i < m_length; i++)
			if (m_characters[i] != ' ' && m_characters[i] != '\t' && m_characters[i] != '\n' && m_characters[i] != '\r')
				return i;
		return -1;
	}

	int32 String::FindFirstSpace(uint32 offset) const
	{
		for (uint32 i = offset; i < m_length; i++)
			if (m_characters[i] == ' ' || m_characters[i] == '\t' || m_characters[i] == '\n' || m_characters[i] == '\r')
				return i;
		return -1;
	}

	String String::RemoveAllOf(const String & str) const
	{
		int32 index = 0;
		String res = "";
		while (index < m_length)
		{
			int32 firstOf = FindFirstOf(str, index);
			if (firstOf == -1)
				break;
			res += SubString(index, firstOf);
			index = firstOf + str.Length();
		}
		res += SubString(index);
		return res;
	}

	String String::TrimBeginning() const
	{
		for (uint32 i = 0; i < m_length; i++)
			if (m_characters[i] != ' ' && m_characters[i] != '\t' && m_characters[i] != '\n' && m_characters[i] != '\r')
				return SubString(i);
	}

	String String::TrimEnding() const
	{
		for (uint32 i = m_length; i >= 0; i--)
			if (m_characters[i] != ' ' && m_characters[i] != '\t' && m_characters[i] != '\n' && m_characters[i] != '\r')
				return SubString(0, i + 1);
	}

	String String::Trim()
	{
		uint32 startIndex = 0;
		uint32 endIndex = m_length;
		for (uint32 i = 0; i < m_length; i++)
		{
			if (m_characters[i] != ' ' && m_characters[i] != '\t' && m_characters[i] != '\n')
			{
				startIndex = i;
				break;
			}
		}

		for (uint32 i = m_length - 1; i >= 0; i--)
		{
			if (m_characters[i] != ' ' && m_characters[i] != '\t' && m_characters[i] != '\n')
			{
				endIndex = i + 1;
				break;
			}
		}
		return SubString(startIndex, endIndex);
	}

	String String::SubString(uint32 startIndex, uint32 endIndex) const
	{
		String substring(endIndex - startIndex);

		for (uint32 i = 0; i < substring.Length(); i++)
		{
			substring[i] = m_characters[startIndex + i];
		}

		return substring;
	}

	String String::SubString(uint32 startIndex) const
	{
		return SubString(startIndex, m_length);
	}

	String String::Cut(uint32 start, uint32 end) const
	{
		String cut(m_length - (end - start + 1));

		for (uint32 i = 0; i < start; i++)
		{
			cut[i] = m_characters[i];
		}

		uint32 counter = start;
		for (uint32 i = end + 1; i < m_length; i++)
		{
			cut[counter++] = m_characters[i];
		}

		return cut;
	}

	bool String::BeginsWith(const String & string) const
	{
		if (m_length < string.Length())
		{
			return false;
		}

		for (uint32 i = 0; i < string.Length(); i++)
		{
			if (m_characters[i] != string[i])
			{
				return false;
			}
		}

		return true;
	}

	bool String::IsEmpty() const
	{
		return m_length == 0;
	}

	char String::operator[](uint32 index) const
	{
		return m_characters[index];
	}

	char & String::operator[](uint32 index)
	{
		return m_characters[index];
	}

	bool String::operator==(const String & string) const
	{
		if (m_length != string.m_length)
			return false;

		for (uint32 i = 0; i < m_length; i++)
			if (m_characters[i] != string[i])
				return false;

		return true;
	}

	bool String::operator<(const String & string) const
	{
		return std::lexicographical_compare(m_characters, m_characters + m_length, string.m_characters, string.m_characters + string.m_length);
	}

	String & String::operator=(const String & string)
	{
		if(m_characters)
			free(m_characters);

		Copy(string);
		return *this;
	}

	String & String::operator=(cstring characters)
	{
		if (m_characters)
			free(m_characters);

		Copy(characters);
		return *this;
	}

	String String::operator+(const String & string) const
	{
		String res(m_length + string.m_length);

		for (uint32 i = 0; i < m_length; i++)
			res[i] = m_characters[i];
		for (uint32 i = 0; i < string.m_length; i++)
			res[m_length + i] = string[i];

		return res;
	}

	String & String::operator+=(const String & string)
	{
		char* newString = new char[m_length + string.m_length + 1];

		for (uint32 i = 0; i < m_length; i++)
			newString[i] = m_characters[i];
		for (uint32 i = 0; i < string.m_length; i++)
			newString[m_length + i] = string[i];

		delete[] m_characters;
		m_characters = newString;
		m_length += string.m_length;
		m_characters[m_length] = '\0';
		return *this;
	}

	void String::Copy(cstring characters)
	{
		m_length = 0;

		while (characters[m_length] != '\0')
		{
			m_length++;
		}

		m_characters = new char[m_length + 1];

		for (uint32 i = 0; i < m_length; i++)
			m_characters[i] = characters[i];

		m_characters[m_length] = '\0';
	}

	void String::Copy(const String & string)
	{
		m_length = string.Length();
		m_characters = new char[m_length + 1];
		for (uint32 i = 0; i < m_length; i++)
		{
			m_characters[i] = string[i];
		}

		m_characters[m_length] = '\0';
	}

	std::ostream & operator<<(std::ostream & out, const String & string)
	{
		out << string.m_characters;
		return out;
	}

}
