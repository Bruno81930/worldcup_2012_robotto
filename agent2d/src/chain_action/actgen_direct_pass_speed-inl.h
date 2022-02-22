#ifndef ACTGEN_DIRECT_PASS_SPEED_H
#define ACTGEN_DIRECT_PASS_SPEED_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../util/FuzzyController-inl.h"

const double kMinDistance = 1.0;
const double kMaxDistance = 21.0;

const int kDistanceInputIndex = 1;
const int kSpeedOutputIndex = 1;

const double kMinSpeed = 0.81;
const double kMaxSpeed = 3.31;

class ActGen_DirectPassSpeed {
public:
	static double getDirectPassSpeed(double distance,
									 double discretization = 8) {
		fuzzy::trimNumeric(&distance,
				          &kMinDistance,
				          &kMaxDistance);

		Instance().directPassSpeed.fuzzify(distance,
										   std::numeric_limits<double>::max());

		return Instance().directPassSpeed.defuzzifyCentroid(kSpeedOutputIndex,
				     	 	 	 	 	 	 	 	 	 	kMinSpeed,
				     	 	 	 	 	 	 	 	 	 	kMaxSpeed,
				     	 	 	 	 	 	 	 	 	 	discretization);
	}
private:
	ActGen_DirectPassSpeed() {
		this->initMembershipFunctions();
		this->initRuleBase();
	}

	// prevent copy-construction
	ActGen_DirectPassSpeed(const ActGen_DirectPassSpeed&);

	// prevent assignment
	ActGen_DirectPassSpeed& operator=(const ActGen_DirectPassSpeed);

	static ActGen_DirectPassSpeed& Instance() {
		static ActGen_DirectPassSpeed singleton;

		return singleton;
	}

	void initMembershipFunctions() {
		directPassSpeed.addMembershipFunction("distanceLow",
				                              kMinDistance,
				                              kMinDistance,
				                              kMinDistance,
				                              kMaxDistance,
				                              kDistanceInputIndex,
				                              fuzzy::INPUT_MEMBERSHIP_FUNCTION);
		directPassSpeed.addMembershipFunction("distanceHigh",
											  kMinDistance,
											  kMaxDistance,
											  kMaxDistance,
											  kMaxDistance,
											  kDistanceInputIndex,
											  fuzzy::INPUT_MEMBERSHIP_FUNCTION);
		directPassSpeed.addMembershipFunction("speedLow",
											  kMinSpeed,
											  kMinSpeed,
											  kMinSpeed,
											  kMaxSpeed,
											  kSpeedOutputIndex,
											  fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		directPassSpeed.addMembershipFunction("speedHigh",
											  kMinSpeed,
											  kMaxSpeed,
											  kMaxSpeed,
											  kMaxSpeed,
											  kSpeedOutputIndex,
											  fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
	}

	void initRuleBase() {
		directPassSpeed.addRule("distanceLow",
								fuzzy::kRuleImplies(),
								"speedLow",
								fuzzy::kRuleEnd());
		directPassSpeed.addRule("distanceHigh",
								fuzzy::kRuleImplies(),
								"speedHigh",
								fuzzy::kRuleEnd());
	}

	fuzzy::FuzzyController<double> directPassSpeed;
};

#endif // #ifndef ACTGEN_DIRECT_PASS_SPEED_H
