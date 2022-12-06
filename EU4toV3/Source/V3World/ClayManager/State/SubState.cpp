#include "SubState.h"
#include "Log.h"
#include "Mappers/CultureMapper/CultureMapper.h"
#include "Mappers/ReligionMapper/ReligionMapper.h"
#include "PoliticalManager/Country/Country.h"
#include "ProvinceManager/PopRatio.h"
#include "State.h"
#include <cmath>
#include <numeric>
#include <ranges>

V3::SubState::SubState(std::shared_ptr<State> theHomeState, ProvinceMap theProvinces): homeState(std::move(theHomeState)), provinces(std::move(theProvinces))
{
}

std::optional<std::string> V3::SubState::getOwnerTag() const
{
	if (owner)
		return owner->getTag();
	return std::nullopt;
}

const std::string& V3::SubState::getHomeStateName() const
{
	if (homeState)
	{
		return homeState->getName();
	}
	else
	{
		static const std::string empty;
		Log(LogLevel::Warning) << "Attempted to access the name of a nullptr state from a substate. Returning empty name.";
		return empty;
	}
}

void V3::SubState::convertDemographics(const ClayManager& clayManager,
	 const mappers::CultureMapper& cultureMapper,
	 const mappers::ReligionMapper& religionMapper,
	 const EU4::CultureLoader& cultureLoader,
	 const EU4::ReligionLoader& religionLoader)
{
	// Just a few sanity checks which should never fail.
	if (!homeState || !owner)
		Log(LogLevel::Error) << "Not converting demographics in insane substate!";

	std::vector<Demographic> newDemographics;
	for (const auto& sourceData: weightedSourceProvinceData | std::views::keys)
	{
		for (const auto& popratio: sourceData.popRatios)
		{
			Demographic newDemo;
			// Religion
			auto religionMatch = religionMapper.getV3Religion(popratio.getReligion());
			if (!religionMatch)
				newDemo.religion = "noreligion";
			else
				newDemo.religion = *religionMatch;

			// Culture
			auto cultureMatch = cultureMapper.cultureMatch(clayManager,
				 cultureLoader,
				 religionLoader,
				 popratio.getCulture(),
				 popratio.getReligion(),
				 homeState->getName(),
				 owner->getTag());
			if (!cultureMatch)
				newDemo.culture = "noculture";
			else
				newDemo.culture = *cultureMatch;

			newDemo.upperRatio = popratio.getUpperRatio();
			newDemo.middleRatio = popratio.getMiddleRatio();
			newDemo.lowerRatio = popratio.getLowerRatio();

			newDemographics.push_back(newDemo);
		}
	}
	demographics.swap(newDemographics);
}

void V3::SubState::generatePops(int totalAmount)
{
	// At this moment we're not concerned with pop types. HOWEVER, demographics do carry a varying amount of ratios,
	// which are (were?) supposed to apply to those types.
	//
	// For now, we'll sum those ratios together and divide by 3, averaging them out.

	if (demographics.empty())
		return;

	// *technically* demoTotal should always be equal 1.
	const auto demoTotal = std::accumulate(demographics.begin(), demographics.end(), 0.0, [](double sum, const auto& demo) {
		return sum + (demo.upperRatio + demo.middleRatio + demo.lowerRatio) / 3;
	});

	// TODO: plug in poptypes via PopPoints.h. If needed.
	for (const auto& demo: demographics)
	{
		const double demoSum = (demo.upperRatio + demo.middleRatio + demo.lowerRatio) / 3;
		auto pop = Pop(demo.culture, demo.religion, "", static_cast<int>(round(static_cast<double>(totalAmount) * demoSum / demoTotal)));
		subStatePops.addPop(pop);
	}
}
