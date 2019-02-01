#pragma once

#include "../DataStructures/String.h"
#include <stdio.h>

namespace Happic {

	class FileReader
	{
	public:
		FileReader(const String& file);
		void Close() const;
		String ReadLine() const;

		static String ReadFile(const String& file, bool include = false);
	private:
		FILE* m_pFile;
	};

}
