#ifndef FUZZY_OFFENSIVE_POS_EVAL_H
#define FUZZY_OFFENSIVE_POS_EVAL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "util/FuzzyController-inl.h"

const double kMinDist            =  0.0;
const double kMaxDistBallPos     = 15.0;
const double kMaxDistOppPos      =  6.0;
const double kMaxDistBallLineOpp =  6.0;
const double kMaxDistCurrPos     = 17.0;
const double kMaxDistOppGoalPos  = 10.0;

const int kDistBallPosInputIndex     = 1;
const int kDistOppPosInputIndex      = 2;
const int kDistBallLineOppInputIndex = 3;
const int kDistCurrPosInputIndex     = 4;
const int kDistOppGoalPosInputIndex  = 5;

const double kMinEval =  0.0;
const double kMaxEval = 20.0;

const int kEvalOutputIndex = 1;

class FuzzyOffensivePosEval {
public:
	static double getOffensivePosEval(double distBallPos,
			                          double distOppPos,
			                          double distBallLineOpp,
			                          double distCurrPos,
			                          double distOppGoalPos,
			                          double discretization = 8) {
		fuzzy::trimNumeric(&distBallPos,
				           &kMinDist,
				           &kMaxDistBallPos);
		fuzzy::trimNumeric(&distOppPos,
					       &kMinDist,
					       &kMaxDistOppPos);
		fuzzy::trimNumeric(&distBallLineOpp,
					       &kMinDist,
					       &kMaxDistBallLineOpp);
		fuzzy::trimNumeric(&distCurrPos,
					       &kMinDist,
					       &kMaxDistCurrPos);
		fuzzy::trimNumeric(&distOppGoalPos,
				           &kMinDist,
				           &kMaxDistOppGoalPos);

		Instance().posEval.fuzzify(distBallPos,
				                   distOppPos,
				                   distBallLineOpp,
				                   distCurrPos,
				                   distOppGoalPos,
				                   std::numeric_limits<double>::max() );

		return Instance().posEval.defuzzifyCentroid(kEvalOutputIndex,
				                                    kMinEval,
				                                    kMaxEval,
				                                    discretization);
	}

private:
	FuzzyOffensivePosEval() {
		this->initMembershipFunctions();
		this->initRuleBase();
	}

	// prevent copy-construction
	FuzzyOffensivePosEval(const FuzzyOffensivePosEval&);

	// prevent assignment
	FuzzyOffensivePosEval& operator=(const FuzzyOffensivePosEval);

	static FuzzyOffensivePosEval& Instance() {
		static FuzzyOffensivePosEval singleton;

		return singleton;
	}

	void initMembershipFunctions() {
		posEval.addMembershipFunction("distBallPosLow",
									  kMinDist,
									  kMinDist,
									  kMinDist,
									  kMaxDistBallPos,
									  kDistBallPosInputIndex,
									  fuzzy::INPUT_MEMBERSHIP_FUNCTION);
		posEval.addMembershipFunction("distBallPosHigh",
									  kMinDist,
									  kMaxDistBallPos,
									  kMaxDistBallPos,
									  kMaxDistBallPos,
									  kDistBallPosInputIndex,
									  fuzzy::INPUT_MEMBERSHIP_FUNCTION);

		posEval.addMembershipFunction("distOppPosLow",
									  kMinDist,
									  kMinDist,
									  kMinDist,
									  kMaxDistOppPos,
									  kDistOppPosInputIndex,
									  fuzzy::INPUT_MEMBERSHIP_FUNCTION);
		posEval.addMembershipFunction("distOppPosHigh",
									  kMinDist,
									  kMaxDistOppPos,
									  kMaxDistOppPos,
									  kMaxDistOppPos,
									  kDistOppPosInputIndex,
									  fuzzy::INPUT_MEMBERSHIP_FUNCTION);

		posEval.addMembershipFunction("distBallLineOppLow",
									  kMinDist,
									  kMinDist,
									  kMinDist,
									  kMaxDistBallLineOpp,
									  kDistBallLineOppInputIndex,
									  fuzzy::INPUT_MEMBERSHIP_FUNCTION);
		posEval.addMembershipFunction("distBallLineOppHigh",
									  kMinDist,
									  kMaxDistBallLineOpp,
									  kMaxDistBallLineOpp,
									  kMaxDistBallLineOpp,
									  kDistBallLineOppInputIndex,
									  fuzzy::INPUT_MEMBERSHIP_FUNCTION);

		posEval.addMembershipFunction("distCurrPosLow",
									  kMinDist,
									  kMinDist,
									  kMinDist,
									  kMaxDistCurrPos,
									  kDistCurrPosInputIndex,
									  fuzzy::INPUT_MEMBERSHIP_FUNCTION);
		posEval.addMembershipFunction("distCurrPosHigh",
									  kMinDist,
									  kMaxDistCurrPos,
									  kMaxDistCurrPos,
									  kMaxDistCurrPos,
									  kDistCurrPosInputIndex,
									  fuzzy::INPUT_MEMBERSHIP_FUNCTION);

		posEval.addMembershipFunction("distOppGoalPosLow",
									  kMinDist,
									  kMinDist,
									  kMinDist,
									  kMaxDistOppGoalPos,
									  kDistOppGoalPosInputIndex,
									  fuzzy::INPUT_MEMBERSHIP_FUNCTION);
		posEval.addMembershipFunction("distOppGoalPosHigh",
									  kMinDist,
									  kMaxDistOppGoalPos,
									  kMaxDistOppGoalPos,
									  kMaxDistOppGoalPos,
									  kDistOppGoalPosInputIndex,
									  fuzzy::INPUT_MEMBERSHIP_FUNCTION);



		posEval.addMembershipFunction("evalLow",
				                      kMinEval,
				                      kMinEval,
				                      kMinEval,
				                      kMaxEval,
				                      kEvalOutputIndex,
									  fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
		posEval.addMembershipFunction("evalHigh",
				                      kMinEval,
				                      kMaxEval,
				                      kMaxEval,
				                      kMaxEval,
				                      kEvalOutputIndex,
									  fuzzy::OUTPUT_MEMBERSHIP_FUNCTION);
	}

	void initRuleBase() {
		posEval.addRule("distBallPosLow",
						fuzzy::kRuleImplies(),
						"evalLow",
						fuzzy::kRuleEnd());
		posEval.addRule("distBallPosHigh",
						fuzzy::kRuleImplies(),
						"evalHigh",
						fuzzy::kRuleEnd());

		posEval.addRule("distOppPosLow",
						fuzzy::kRuleImplies(),
						"evalLow",
						fuzzy::kRuleEnd());
		posEval.addRule("distOppPosHigh",
						fuzzy::kRuleImplies(),
						"evalHigh",
						fuzzy::kRuleEnd());

		posEval.addRule("distBallLineOppLow",
						fuzzy::kRuleImplies(),
						"evalLow",
						fuzzy::kRuleEnd());
		posEval.addRule("distBallLineOppHigh",
						fuzzy::kRuleImplies(),
						"evalHigh",
						fuzzy::kRuleEnd());

		posEval.addRule("distCurrPosLow",
						fuzzy::kRuleImplies(),
						"evalHigh",
						fuzzy::kRuleEnd());
		posEval.addRule("distCurrPosHigh",
						fuzzy::kRuleImplies(),
						"evalLow",
						fuzzy::kRuleEnd());

		posEval.addRule("distOppGoalPosLow",
						fuzzy::kRuleImplies(),
						"evalHigh",
						fuzzy::kRuleEnd());
		posEval.addRule("distOppGoalPosHigh",
						fuzzy::kRuleImplies(),
						"evalLow",
						fuzzy::kRuleEnd());
	}

	fuzzy::FuzzyController<double> posEval;
};

#endif
