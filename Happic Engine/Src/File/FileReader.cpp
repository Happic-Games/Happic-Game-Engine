#include "FileReader.h"

namespace Happic {

	FileReader::FileReader(const String & file)
	{
		m_pFile = fopen(file.C_Str(), "r");
	}

	void FileReader::Close() const
	{
		fclose(m_pFile);
	}

	String FileReader::ReadLine() const
	{
		char line[1000];
		if (fgets(line, 1000, m_pFile))
		{
			return String(line);
		}
		else
		{
			return "";
		}
	}

	String FileReader::ReadFile(const String & file, bool include)
	{
		FileReader reader(file);
		String text = "";
		String line = "";

		while (!(line = reader.ReadLine()).IsEmpty())
		{
			if (include)
			{
				if (line.BeginsWith("#include "))
				{
					String includeFile = line.SubString(9, line.Length());

					if (includeFile[includeFile.Length() - 1] == '\n')
					{
						includeFile = includeFile.Cut(includeFile.Length(), includeFile.Length());
					}

					text += ReadFile(includeFile, true);
					continue;
				}
			}

			text += line;
		}

		reader.Close();
		return text;
	}

}