#ifndef CULTURE_GROUP_PARSER
#define CULTURE_GROUP_PARSER
#include "CultureParser.h"
#include "Parser.h"
#include <map>

namespace EU4
{
class CultureGroupParser: commonItems::parser
{
  public:
	CultureGroupParser() = default;
	CultureGroupParser(std::string theName, std::istream& theStream);

	[[nodiscard]] const auto& getName() const { return cultureGroupName; }
	[[nodiscard]] const auto& getCultures() const { return cultures; }

	void mergeCulture(const std::string& cultureName, const CultureParser& cultureParser);

  private:
	void registerKeys();

	std::string cultureGroupName;
	std::map<std::string, CultureParser> cultures;

	std::vector<std::string> maleNames;
	std::vector<std::string> femaleNames;
	std::vector<std::string> dynastyNames;
};
} // namespace EU4

#endif // CULTURE_GROUP_PARSER
