#ifndef FUZZY_HETRO_MATCHER_H
#define FUZZY_HETRO_MATCHER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "util/FuzzyController-inl.h"

#include <vector>

const double kMinAim           =  0.0;
const double kMaxAim           = 10.0;
const double kMinCenterDefense =  0.0;
const double kMaxCenterDefense = 10.0;
const double kMinSideDefense   =  0.0;
const double kMaxSideDefense   = 10.0;
const double kMinCenterAttack  =  0.0;
const double kMaxCenterAttack  = 10.0;
const double kMinSideAttack    =  0.0;
const double kMaxSideAttack    = 10.0;
const double kMinThroughPass   =  0.0;
const double kMaxThroughPass   = 10.0;

const int kAimInputIndex           = 1;
const int kCenterDefenseInputIndex = 2;
const int kSideDefenseInputIndex   = 3;
const int kCenterAttackInputIndex  = 4;
const int kSideAttackInputIndex    = 5;
const int kThroughPassInputIndex   = 6;

//output weights (usually 0-10)
const double kMinWeight =  0.0;
const double kMaxWeight = 10.0;

const int kCenterBackSpeedOutputIndex = 1;
const int kCenterBackStaminaOutputIndex = 2;
const int kCenterBackKickOutputIndex = 3;

const int kSideBackSpeedOutputIndex = 4;
const int kSideBackStaminaOutputIndex = 5;
const int kSideBackKickOutputIndex = 6;

const int kHalfSpeedOutputIndex = 7;
const int kHalfStaminaOutputIndex = 8;
const int kHalfKickOutputIndex = 9;

const int kCenterForwardSpeedOutputIndex = 10;
const int kCenterForwardStaminaOutputIndex = 11;
const int kCenterForwardKickOutputIndex = 12;

const int kSideForwardSpeedOutputIndex = 13;
const int kSideForwardStaminaOutputIndex = 14;
const int kSideForwardKickOutputIndex = 15;

const int kCenterBackIndex    = 0;
const int kSideBackIndex      = 1;
const int kHalfIndex          = 2;
const int kCenterForwardIndex = 3;
const int kSideForwardIndex   = 4;

const int kSpeedIndex   = 0;
const int kStaminaIndex = 1;
const int kKickIndex    = 2;

class FuzzyHetroMatcher {
public:
	static std::vector<std::vector<double> > getHetroWeights(double aim,
			                                                 double centerDefense,
			                                                 double sideDefense,
			                                                 double centerAttack,
			                                                 double sideAttack,
			                                                 double throughPass,
			                                                 double discretization = 8) {
		Instance().hetroWeights.fuzzify(aim,
				                        centerDefense,
				                        sideDefense,
				                        centerAttack,
				                        sideAttack,
				                        throughPass,
				                        std::numeric_limits<double>::max());

		std::vector<std::vector<double> > result;

		result.resize(5);

		for (int i = 0; i < 5; i ++) {
			result[i].resize(4);
		}

		int outputIndex = 1;

		for (int i = 0; i < 5; i ++)
		{
			for (int j = 0; j < 3; j ++)
			{
				result[i][j] = Instance().hetroWeights.defuzzifyCentroid(outputIndex,
						                                                 kMinWeight,
						                                                 kMaxWeight,
						                                                 discretization);
				outputIndex += 1;
			}
		}

		return result;
	}
private:
	FuzzyHetroMatcher() {
		this->initMembershipFunctions();
		this->initRuleBase();
	}

	// prevent copy-construction
	FuzzyHetroMatcher(const FuzzyHetroMatcher&);

	// prevent assignment
	FuzzyHetroMatcher& operator=(const FuzzyHetroMatcher);

	static FuzzyHetroMatcher& Instance() {
		static FuzzyHetroMatcher singleton;

		return singleton;
	}

	void initMembershipFunctions() {
		hetroWeights.addMembershipFunction("aimLow",
										   kMinAim,
										   kMinAim,
										   kMinAim,
										   kMaxAim,
										   kAimInputIndex,
										   fuzzy::INPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("aimHigh",
										   kMinAim,
										   kMaxAim,
										   kMaxAim,
										   kMaxAim,
										   kAimInputIndex,
										   fuzzy::INPUT_MEMBERSHIP_FUNCTION);

		hetroWeights.addMembershipFunction("centerDefenseLow",
										   kMinCenterDefense,
										   kMinCenterDefense,
										   kMinCenterDefense,
										   kMaxCenterDefense,
										   kCenterDefenseInputIndex,
										   fuzzy::INPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("centerDefenseHigh",
										   kMinCenterDefense,
										   kMaxCenterDefense,
										   kMaxCenterDefense,
										   kMaxCenterDefense,
										   kCenterDefenseInputIndex,
										   fuzzy::INPUT_MEMBERSHIP_FUNCTION);

		hetroWeights.addMembershipFunction("sideDefenseLow",
										   kMinSideDefense,
										   kMinSideDefense,
										   kMinSideDefense,
										   kMaxSideDefense,
										   kSideDefenseInputIndex,
										   fuzzy::INPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("sideDefenseHigh",
										   kMinSideDefense,
										   kMaxSideDefense,
										   kMaxSideDefense,
										   kMaxSideDefense,
										   kSideDefenseInputIndex,
										   fuzzy::INPUT_MEMBERSHIP_FUNCTION);

		hetroWeights.addMembershipFunction("centerAttackLow",
										   kMinCenterAttack,
										   kMinCenterAttack,
										   kMinCenterAttack,
										   kMaxCenterAttack,
										   kCenterAttackInputIndex,
										   fuzzy::INPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("centerAttackHigh",
										   kMinCenterAttack,
										   kMaxCenterAttack,
										   kMaxCenterAttack,
										   kMaxCenterAttack,
										   kCenterAttackInputIndex,
										   fuzzy::INPUT_MEMBERSHIP_FUNCTION);

		hetroWeights.addMembershipFunction("sideAttackLow",
										   kMinSideAttack,
										   kMinSideAttack,
										   kMinSideAttack,
										   kMaxSideAttack,
										   kSideAttackInputIndex,
										   fuzzy::INPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("sideAttackHigh",
										   kMinSideAttack,
										   kMaxSideAttack,
										   kMaxSideAttack,
										   kMaxSideAttack,
										   kSideAttackInputIndex,
										   fuzzy::INPUT_MEMBERSHIP_FUNCTION);

		hetroWeights.addMembershipFunction("throughPassLow",
										   kMinThroughPass,
										   kMinThroughPass,
										   kMinThroughPass,
										   kMaxThroughPass,
										   kThroughPassInputIndex,
										   fuzzy::INPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("throughPassHigh",
										   kMinThroughPass,
										   kMaxThroughPass,
										   kMaxThroughPass,
										   kMaxThroughPass,
										   kThroughPassInputIndex,
										   fuzzy::INPUT_MEMBERSHIP_FUNCTION);



		hetroWeights.addMembershipFunction("centerBackSpeedLow",
										   kMinWeight,
										   kMinWeight,
										   kMinWeight,
										   kMaxWeight,
										   kCenterBackSpeedOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("centerBackSpeedHigh",
										   kMinWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kCenterBackSpeedOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("centerBackStaminaLow",
										   kMinWeight,
										   kMinWeight,
										   kMinWeight,
										   kMaxWeight,
										   kCenterBackStaminaOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("centerBackStaminaHigh",
										   kMinWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kCenterBackStaminaOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("centerBackKickLow",
										   kMinWeight,
										   kMinWeight,
										   kMinWeight,
										   kMaxWeight,
										   kCenterBackKickOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("centerBackKickHigh",
										   kMinWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kCenterBackKickOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);

		hetroWeights.addMembershipFunction("sideBackSpeedLow",
										   kMinWeight,
										   kMinWeight,
										   kMinWeight,
										   kMaxWeight,
										   kSideBackSpeedOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("sideBackSpeedHigh",
										   kMinWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kSideBackSpeedOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("sideBackStaminaLow",
										   kMinWeight,
										   kMinWeight,
										   kMinWeight,
										   kMaxWeight,
										   kSideBackStaminaOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("sideBackStaminaHigh",
										   kMinWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kSideBackStaminaOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("sideBackKickLow",
										   kMinWeight,
										   kMinWeight,
										   kMinWeight,
										   kMaxWeight,
										   kSideBackKickOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("sideBackKickHigh",
										   kMinWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kSideBackKickOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);

		hetroWeights.addMembershipFunction("halfSpeedLow",
										   kMinWeight,
										   kMinWeight,
										   kMinWeight,
										   kMaxWeight,
										   kHalfSpeedOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("halfSpeedHigh",
										   kMinWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kHalfSpeedOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("halfBackStaminaLow",
										   kMinWeight,
										   kMinWeight,
										   kMinWeight,
										   kMaxWeight,
										   kHalfStaminaOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("halfStaminaHigh",
										   kMinWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kHalfStaminaOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("halfKickLow",
										   kMinWeight,
										   kMinWeight,
										   kMinWeight,
										   kMaxWeight,
										   kHalfKickOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("halfKickHigh",
										   kMinWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kHalfKickOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);

		hetroWeights.addMembershipFunction("centerForwardSpeedLow",
										   kMinWeight,
										   kMinWeight,
										   kMinWeight,
										   kMaxWeight,
										   kCenterForwardSpeedOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("centerForwardSpeedHigh",
										   kMinWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kCenterForwardSpeedOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("centerForwardStaminaLow",
										   kMinWeight,
										   kMinWeight,
										   kMinWeight,
										   kMaxWeight,
										   kCenterForwardStaminaOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("centerForwardStaminaHigh",
										   kMinWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kCenterForwardStaminaOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("centerForwardKickLow",
										   kMinWeight,
										   kMinWeight,
										   kMinWeight,
										   kMaxWeight,
										   kCenterForwardKickOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("centerForwardKickHigh",
										   kMinWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kCenterForwardKickOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);

		hetroWeights.addMembershipFunction("sideForwardSpeedLow",
										   kMinWeight,
										   kMinWeight,
										   kMinWeight,
										   kMaxWeight,
										   kSideForwardSpeedOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("sideForwardSpeedHigh",
										   kMinWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kSideForwardSpeedOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("sideForwardStaminaLow",
										   kMinWeight,
										   kMinWeight,
										   kMinWeight,
										   kMaxWeight,
										   kSideForwardStaminaOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("sideForwardStaminaHigh",
										   kMinWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kSideForwardStaminaOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("sideForwardKickLow",
										   kMinWeight,
										   kMinWeight,
										   kMinWeight,
										   kMaxWeight,
										   kSideForwardKickOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		hetroWeights.addMembershipFunction("sideForwardKickHigh",
										   kMinWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kMaxWeight,
										   kSideForwardKickOutputIndex,
										   fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
	}

	void initRuleBase() {
		hetroWeights.addRule("aimLow",
				 			 fuzzy::kRuleImplies(),
							 "centerBackSpeedHigh",
							 "centerBackStaminaHigh",
							 "centerBackKickHigh",
							 "sideBackSpeedHigh",
							 "sideBackStaminaHigh",
							 "sideBackKickHigh",
							 "halfStaminaHigh",
							 "centerForwardSpeedLow",
							 "centerForwardStaminaLow",
							 "centerForwardKickLow",
							 "sideForwardSpeedLow",
							 "sideForwardStaminaLow",
							 "sideForwardKickLow",
							 fuzzy::kRuleEnd());
		hetroWeights.addRule("aimHigh",
						 	 fuzzy::kRuleImplies(),
							 "centerBackSpeedLow",
							 "centerBackStaminaLow",
							 "centerBackKickLow",
							 "sideBackSpeedLow",
							 "sideBackStaminaLow",
							 "sideBackKickLow",
							 "halfStaminaHigh",
							 "centerForwardSpeedHigh",
							 "centerForwardStaminaHigh",
							 "centerForwardKickHigh",
							 "sideForwardSpeedHigh",
							 "sideForwardStaminaHigh",
							 "sideForwardKickHigh",
							 fuzzy::kRuleEnd());

		hetroWeights.addRule("centerDefenseLow",
				             fuzzy::kRuleImplies(),
				             "centerForwardSpeedLow",
				             "centerForwardKickLow",
				             fuzzy::kRuleEnd());
		hetroWeights.addRule("centerDefenseHigh",
							 fuzzy::kRuleImplies(),
							 "centerForwardSpeedHigh",
							 "centerForwardKickHigh",
							 fuzzy::kRuleEnd());

		hetroWeights.addRule("sideDefenseLow",
							 fuzzy::kRuleImplies(),
							 "sideForwardSpeedLow",
							 "sideForwardKickLow",
							 fuzzy::kRuleEnd());
		hetroWeights.addRule("sideDefenseHigh",
							 fuzzy::kRuleImplies(),
							 "sideForwardSpeedHigh",
							 "sideForwardKickHigh",
							 fuzzy::kRuleEnd());

		hetroWeights.addRule("centerAttackLow",
							 fuzzy::kRuleImplies(),
							 "centerBackSpeedLow",
							 "centerBackKickLow",
							 fuzzy::kRuleEnd());
		hetroWeights.addRule("centerAttackHigh",
							 fuzzy::kRuleImplies(),
							 "centerBackSpeedHigh",
							 "centerBackKickHigh",
							 fuzzy::kRuleEnd());

		hetroWeights.addRule("sideAttackLow",
							 fuzzy::kRuleImplies(),
							 "sideBackSpeedLow",
							 "sideBackKickLow",
							 fuzzy::kRuleEnd());
		hetroWeights.addRule("sideAttackHigh",
							 fuzzy::kRuleImplies(),
							 "sideBackSpeedHigh",
							 "sideBackKickHigh",
							 fuzzy::kRuleEnd());

		hetroWeights.addRule("throughPassHigh",
							 fuzzy::kRuleImplies(),
							 "centerBackSpeedHigh",
							 "centerBackKickHigh",
							 fuzzy::kRuleEnd());
	}

	fuzzy::FuzzyController<double> hetroWeights;
};

#endif // #ifndef FUZZY_HETRO_MATCHER_H
