/**
 * @brief
 * This code file represents a generic fuzzy logic controller
 * Code is divided into 3 classes: MembershipFunction, Rule and FuzzyController
 * In a nutshell:
 *   MembershipFunction represents any input/output in terms of
 *   its increase and decrease of strength on one or more levels (levels can
 *   be something like low, medium, high, etc ...). Rule is a combination of
 *   some MembershipFunction (Ms) where a rule is simply
 *   IF M1 AND M2 THEN M5 AND M8. FuzzyController is the class responsible
 *   for managing the relation between MembershipFunction and Rule,
 *   processing the input(s) to the Fuzzy Controller and calculating the
 *   output.
 * */

#ifndef UTILITIES_FUZZYCONTROLLER_H
#define UTILITIES_FUZZYCONTROLLER_H

#include <vector>
#include <hash_map>
#include <string.h>
#include <stdarg.h>
#include <limits>
#include <iostream>
#include <algorithm>

namespace fuzzy {

/**
 * @brief
 * Function name: trimNumeric
 * Sometimes the range for a group of membership functions R representing
 *   an input I is smaller than the possible range for I. So instances of
 *   I that are smaller than R are set to Rmin and those that are larger
 *   than R are set to Rmax.
 * It's important to make sure that all values fall within the range because
 *   values falling outside it always give a fuzzy strength = 0 which doesn't
 *   comply with the logic of the controller most of the time (don't trim
 *   ONLY if you are sure about what you are doing).
 * Since the controller has some fixed datatypes (int and double) and some
 *   variable ones defined by the template, the +1 and -1 are used with the
 *   limits to avoid any problem(s) that may result from runtime casting.
 * @param numeric: the number to be trimed, this parameter is an input and
 *                 output at the same time as the original instance of the
 *                 number is modified rather than taking a copy and returning
 *                 the new value.
 * @param lowerLimit: the lowest possible value for numeric
 * @param upperLimit: the highest possible value for numeric
 */
template<class NumericalType>
void trimNumeric(NumericalType* numeric,
		         const NumericalType* lowerLimit,
		         const NumericalType* upperLimit) {
  if (*numeric < (*lowerLimit + 1)) {
    *numeric = (*lowerLimit + 1);
  } else if (*numeric > (*upperLimit - 1)) {
    *numeric = (*upperLimit - 1);
  }
}

/**
 * @brief
 * The add rule function is generic in terms of accepting any
 *   number of antecedents and consequents by taking the "..."
 *   argument, so these constants must be passed to the function
 *   for it to be able to detect end of antecedents and end of
 *   consequents
 * */
//const char* kRuleImplies = "__IMPLIES__";
//const char* kRuleEnd     = "__END_RULE__";
inline
const char* kRuleImplies() {
	return "__IMPLIES__";
}

inline
const char* kRuleEnd() {
	return "__END_RULE__";
}

/**
 * @brief
 * The fuzzification function takes the "..." argument as an input
 *   for it to be able to accept any number of numerical inputs for
 *   a fuzzy controller.
 * In order to identify which input goes to which membership function,
 *   each membership function has an attribute "fuzzificationIndex"
 *   denoting which fuzzification input should fuzzify it.
 * Since rules can differ in the count and order of consequents,
 *   fuzzification index of the output membership function to be
 *   deffuzified is passed to the defuzzification function to point
 *   to the correct output membership function.
 * The fuzzification index value starts with 1, the default 0 value
 *   leads to ignoring the corresponding membership function, this can
 *   be useful to easily eliminate membership function(s) during testing
 *   or training.
 * */
const int kDummyFuzzificationIndex    = 0;
const int kMinValidFuzzificationIndex = 1;

/**
 * @brief
 * Any membership function can either belong to input membership functions or
 *   output membership functions.
 * The MembershipFunctionKind is used to differentiate between input and output
 *   membership function in generic functions used by both.
 * */
enum MembershipFunctionKind {
  INPUT_MEMBERSHIP_FUNCTION,
  OUTPUT_MEMBERSHIP_FUNCTION
};

/**
 * @brief
 * Forward declaration because:
 *   1- Rule and FuzzyController are friends of MembershipFunction
 *   2- FuzzyController is friend of Rule
 * Friendship is used to allow access of private functions
 * */
template <class MembershipType> class Rule;
template <class MembershipType> class FuzzyController;

/**
 * @brief
 * MembershipFunction is a way to represent a subset of any input or output
 *   for a Fuzzy Controller (e.g.: speed_slow, speed_fast, weather_hot,
 *   weather_cold, etc ...).
 * MembershipFunction only represents triangular and trapezoidal functions
 *   where the y-coordinate of start and end is always 0 while the one for
 *   topLeft and topRight is always 1
 * This design should suffice, however it can be modified easily; modification
 *   may need modifying the addRule function in the FuzzyController class
 * This design also reduces the runtime as it's a lot easier and faster to
 *   compute the fuzzyStrength for triangular and trapizoidal shapes
 * */
template <class MembershipType>
class MembershipFunction {
  public:
    /**
		 * @brief
     * fuzzificationIndex is useful to give each group of membership functions
     *   (e.g.: low, medium and high of speed) an index to define which input
     *   from the fuzzify function to be used with it.
     * */
    MembershipFunction(void) {
      this->start    = 0;
      this->topLeft  = 0;
      this->topRight = 0;
      this->end      = 0;

      this->fuzzificationIndex = kDummyFuzzificationIndex;

      this->fuzzyStrength = 0;
    }

    /**
     * @param start: represents the start point on the x-axis for the
     *               membership function and its y-axis co-ordinate value
     *               is 0
     * @param topLeft: its value is >= start and its y-axis co-ordinate
     *                 value is 1
     * @param topRight: its value it >= topLeft and its y-axis co-ordinate
     *                  value is 1
     * @param end: its value is >= topRight and its y-axis co-ordinate
     *             value is 0
     * @param fuzzificationIndex: the index (1-N) of the corresponding input
     *                            passed to the fuzzification function in
     *                            the Fuzzy Controller and to the
     *                            defuzzification function as well to define
     *                            which group of output membership functions
     *                            to work on
     * */
    MembershipFunction(MembershipType start,
                       MembershipType topLeft,
                       MembershipType topRight,
                       MembershipType end,
                       int fuzzificationIndex) {
      this->start    = start;
      this->topLeft  = topLeft;
      this->topRight = topRight;
      this->end      = end;

      this->fuzzificationIndex = fuzzificationIndex;

      this->fuzzyStrength = 0;
    }

  private:
    friend class Rule<MembershipType>;
    friend class FuzzyController<MembershipType>;

    /*
     * This function is mainly useful in making the controller dynamic in
     *   a way that allows trying multiple values to get the desired results.
     * This should be useful for training purposes.
     * The following setters are for the same purpose.
     * */
    void updateMembershipFunction(MembershipType start,
                                  MembershipType topLeft,
                                  MembershipType topRight,
                                  MembershipType end) {
      this->start    = start;
      this->topLeft  = topLeft;
      this->topRight = topRight;
      this->end      = end;

      this->fuzzyStrength = 0;
    }

    void setStart(const MembershipType start) {
      this->start = start;

      this->fuzzyStrength = 0;
    }

    void setTopLeft(const MembershipType topLeft) {
      this->topLeft = topLeft;

      this->fuzzyStrength = 0;
    }

    void setTopRight(const MembershipType topRight) {
      this->topRight = topRight;

      this->fuzzyStrength = 0;
    }

    void setEnd(const MembershipType end) {
      this->end = end;

      this->fuzzyStrength = 0;
    }

    void setFuzzificationIndex(const int fuzzificationIndex) {
      this->fuzzificationIndex = fuzzificationIndex;
    }

    const int* getFuzzificationIndex(void) const {
      return &this->fuzzificationIndex;
    }

    double* getFuzzyStrength(void) {
      return &this->fuzzyStrength;
    }

    /*
     * Given anyinput the fuzzy strength is set to a value ranging from
     *   0.0 to 1.0 where this output represents the percentage of how
     *   the input represents this membership function
     * Typecasting is used since this is a template class that can
     *   accept types like int while the fuzzy strength value must be able
     *   to fall anywhere between 0.00 and 1.00 otherwise (making it 0 or 1)
     *   will make it act like a crisp controller rather than a fuzzy one
     *   and lose the majority of its benefit.
     * */
    void calculateFuzzyStrength(MembershipType &input) {
      if (input < this->start || input > this->end) {
        this->fuzzyStrength = 0;
      } else if (input >= this->topLeft && input <= this->topRight) {
        this->fuzzyStrength = 1;
      } else if (input < this->topLeft) {
        this->fuzzyStrength = (double) ((double)(input - this->start) /
                                        (double)(this->topLeft - this->start));
      } else if (input > this->topRight) {
        this->fuzzyStrength = (double) ((double)(this->end - input) /
                                        (double)(this->end - this->topRight));
      }
    }

    MembershipType start,
                   topLeft,
                   topRight,
                   end;

    double fuzzyStrength;

    int fuzzificationIndex;
};

/**
 * @brief
 * A rule is a group of one or more input membership functions (antecedents)
 *   that implies a group of one or more output membership functions
 *   (consequents).
 * This is where the logic of the Fuzzy Controller is represented simply
 *   in natural language (e.g.: IF weather_cold AND sky_raining THEN
 *   close_window AND turn_heater_high).
 * Similar to the fact that a Membership Function has a fuzzy strength
 *   depending on the corresponding input's value, a rule has firing
 *   strength depending on the combined fuzzy strengths of its
 *   antecedents (usually the min fuzzy strength of its antecedents)
 * */
template <class MembershipType>
class Rule {
  public:
    /**
		 * @brief
     * Rules point to membership functions rather than taking a copy
     *   because the same membership function can be used in multiple rules
     *   while a change in value on the membership side should reflect
     *   on the rule without the need for any changes in the rule.
     * */
    Rule(std::vector<MembershipFunction<MembershipType>*> antecedents,
         std::vector<MembershipFunction<MembershipType>*> consequents) {
      this->antecedents = antecedents;
      this->consequents = consequents;

      this->antecedentsCount = antecedents.size();
      this->consequentsCount = consequents.size();

      this->firingStrength = 0;
    }

  private:
    friend class FuzzyController<MembershipType>;

    /*
     * The output strength is useful to set the upper limit for the
     *   contribution of any output value in the final result. Let's say
     *   we have possible output values O1-On and corresponding output
     *   strengths S1-Sn where the output function is similar to
     *   Output = S1*O1 + ... + Sn*On / S1 + ... + Sn where each Oi
     *   correspods to an output membership function, so the following
     *   function is responsible for calculating the corresponding Si.
     *   Si is simply the minimum of the the fuzzy strength and the
     *   corresponding rule's firing strength. In case the output is
     *   included in more than one rule, the maximum outputStrength is
     *   considered.
     * @param consequentIndex: represents the order of the output in the
     *                         rule (e.g.: if we want Y in the rule
     *                         A and B --> X and Y then the index is 1)
     * @param input: is any Oi according to the above explanation,
     *               it's called input because it's an input from the
     *               output membership function's prespective; however it's
     *               still an output (consequent) from the Fuzzy Controller's
     *               prespective
     * */
    double getOutputStrength(const unsigned int consequentIndex,
                             MembershipType input) const {
      for (std::size_t i = 0; i < consequentsCount; i++) {
        if (*(this->consequents[i]->getFuzzificationIndex()) ==
              consequentIndex) {
          this->consequents[i]->calculateFuzzyStrength(input);
          return std::min(this->firingStrength,
                          *(this->consequents[i]->getFuzzyStrength()));
        }
      }

      // default return
      return 0.0;
    }

    /*
     * calculateFiringStrength function is responsible for calculating
     *   the firing strength of the rule by getting the min. of its
     *   antecedents' fuzzy strengths.
     * */
    void calculateFiringStrength(void) {
      this->firingStrength = *(this->antecedents[0]->getFuzzyStrength());

      for (std::size_t i = 1; i < antecedentsCount; i ++) {
        this->firingStrength =
          std::min(this->firingStrength,
                   *(this->antecedents[i]->getFuzzyStrength()));
      }
    }

    std::vector<MembershipFunction<MembershipType>*> antecedents,
                                                     consequents;

    std::size_t antecedentsCount,
                consequentsCount;

    double firingStrength;
};

/**
 * @brief
 * Fuzzy Controller is the class the contains the fuzzification function,
 *   the rulebase and the defuzzification function; those 3 main components
 *   deal with and use stored objects from the MembershipFunction and the
 *   Rule classes.
 * MembershipFunction objects are stored in a hash table where the 'name'
 *   (describing the functionality and type of the MembershipFunction) is
 *   the key. Hash table is used since upon the creation of rules,
 *   MembershipFunction objects are pointed to multiple times.
 * Rules are stored in a vector since all rule-related operations involve
 *   looping on all the available rules.
 * */
template <class MembershipType>
class FuzzyController {
  public:
    FuzzyController(void) {
      this->rulesCount = 0;
    }
    /**
		 * @brief
     * addMembershipFunction simply adds a new Membership Function to the
     *   hash table
     * @param name: is used as a key to the membership function when storing
     *              it in the hash table
     * @param start, topLeft, topRight and end: represent the x-axis points
     *              of the membership function from the smallest to the largest
     *              as explained in the MembershipFunction class
     * @param fuzzificationIndex: defines the order of the Memership Functions
     *                            group that this one belongs to.
     * @param membershipFunctionKind: defines whether this membership function
     *                                belongs to input or output membership
     *                                functions
     * */
    void addMembershipFunction(const char* name,
                               MembershipType start,
                               MembershipType topLeft,
                               MembershipType topRight,
                               MembershipType end,
                               int fuzzificationIndex,
                               MembershipFunctionKind membershipFunctionKind) {
      if (membershipFunctionKind == INPUT_MEMBERSHIP_FUNCTION) {
        inputMembershipFunctions[name] =
          MembershipFunction<MembershipType>(start,
                                             topLeft,
                                             topRight,
                                             end,
                                             fuzzificationIndex);
      } else if (membershipFunctionKind == OUTPUT_MEMBERSHIP_FUNCTION) {
        outputMembershipFunctions[name] =
          MembershipFunction<MembershipType>(start,
                                             topLeft,
                                             topRight,
                                             end,
                                             fuzzificationIndex);
      }
    }

    /**
		 * @brief
     * addRule function MUST get at least one antecedent and one
     *   concequent.
     * The function accepts the '...' paramter in order to allow for a rule
     *   with any number of antecedents/concequents.
     * An antecedent or concequent is represented by the name of the
     *   corresponding Membership Function.
     * In order to define the end of antecedents and concequents in the input
     *   parameters, 'kRuleImplies' constant MUST be passed after the
     *   antecedents and 'kRuleEnd' contant MUST be passed after the
     *   concequents.
     * Example: addRule("watherHot",
     *                  "sunny",
     *                  kRuleImplies,
     *                  "curtainsDown",
     *                  "coolerOn",
     *                  kRuleEnd);
     * */
    void addRule(const char* firstAntecedent, ...) {
      antecedents.clear();
      consequents.clear();

      va_list argsPointer;
      va_start(argsPointer, firstAntecedent);

      const char* currentArg = firstAntecedent;

      while (strcmp(currentArg, kRuleImplies()) != 0) {
        antecedents.push_back
          (&inputMembershipFunctions.find(currentArg)->second);
        currentArg = va_arg(argsPointer, const char*);
      }

      currentArg = va_arg(argsPointer, const char*);

      while (strcmp(currentArg, kRuleEnd()) != 0) {
        consequents.push_back
          (&outputMembershipFunctions.find(currentArg)->second);
        currentArg = va_arg(argsPointer, const char*);
      }

      va_end(argsPointer);

      rules.push_back(Rule<MembershipType>(antecedents, consequents));

      this->rulesCount += 1;
    }

    /**
		 * @brief
     * After all membership functions and rules are added to the fuzzy
     *   controller, the the controller is ready to function. On the cycle
     *   of passing an input and getting an output, fuzzify is the 1st step.
     * The functionality of this function is to accept some inputs, and
     *   calculate the fuzzy strength of membership function(s) corresponding
     *   to each of the inputs, where the order of the input (0 - N) binds
     *   this input with the corresponding membership function(s).
     * The '...' parameter is passed to the function to allow for the
     *   flexibility of passing any number of inputs,
     *   numeric_limits<MembershipType>::max() MUST be the last parameter
     *   to be passed to the function to mark the end of input.
     * Example (assuming MembershipType = float and assuming 3 groups of
     *   input membership functions):
     *   fuzzify(20.0,
     *           12.3,
     *           44.7,
     *           numeric_limits<float>::max());
     * NOTE: this function won't behave correctly in case one of the parameters
     *       was equivalent to std::numeric_limits<MembershipType>::max()
     *       because std::numeric_limits<MembershipType>::max() is used to
     *       mark the end of input for the function. Usually this mistake
     *       doesn't happen.
     * */
    void fuzzify(MembershipType firstInput, ...) {
      MembershipType currentInput = firstInput;

      va_list argsPointer;
      va_start(argsPointer, firstInput);

      for (int inputIndex = kMinValidFuzzificationIndex;
           (currentInput != std::numeric_limits<MembershipType>::max()) &&
           (inputIndex != std::numeric_limits<int>::max());
           inputIndex ++) {
        for (membershipFunctionsIterator =  inputMembershipFunctions.begin();
             membershipFunctionsIterator != inputMembershipFunctions.end  ();
             membershipFunctionsIterator ++){
          if (*(membershipFunctionsIterator->second.getFuzzificationIndex()) ==
              inputIndex) {
            membershipFunctionsIterator->second.calculateFuzzyStrength(
              currentInput);
          }
        }

        currentInput = va_arg(argsPointer, MembershipType);
      }

      va_end(argsPointer);
    }

    /**
		 * @brief
     * defuzzifyCentroid function is responsible for calculating the output
     *   of the Fuzzy Controller for some combination of inputs.
     * @param consequentIndex: defines which group of output membership
     *                         functions is to resemble the output (for
     *                         Fuzzy Controllers with multiple outputs N,
     *                         this function has to be called N time for each
     *                         output).
     * @param consequentStart: defines the minimum value for the output (output
     *                         can still be 0) (normally it is the start point
     *                         of the minimum membership function representing
     *                         the output).
     * @param consequentEnd: same as consequentStart but for the maximum
     * @param discretization: defines how many discrete points should be used
     *                        during output calculation
     * The consequentStart and consequentEnd parameters gives flexibility
     *   in defining the output limits in a dynamic real-time way.
     * */
    MembershipType defuzzifyCentroid(unsigned int consequentIndex,
                                     MembershipType consequentStart,
                                     MembershipType consequentEnd,
                                     MembershipType discretization
                                       = 8) {
      for (std::size_t i = 0; i < rulesCount; i ++) {
        this->rules[i].calculateFiringStrength();
      }

      MembershipType discretizationDelta = (consequentEnd - consequentStart) /
                                           discretization;

      double currentOutputStrength,
             numerator,
             dominator;

      numerator = 0.0;
      dominator = 0.0;

      for (MembershipType i = consequentStart;
           i <= consequentEnd;
           i += discretizationDelta) {
        currentOutputStrength = 0.0;
        for (std::size_t j = 0; j < rulesCount; j ++) {
          currentOutputStrength =
            std::max(currentOutputStrength,
                     this->rules[j].getOutputStrength(consequentIndex, i));
        }
        // This part gives more strength for discrete points with
        //   higher outputStrength (>0.5) than those that are lower
        /*currentOutputStrength = currentOutputStrength > 0.5?
                                  currentOutputStrength*2 :
                                  currentOutputStrength/2;*/
        numerator += i * currentOutputStrength;
        dominator +=     currentOutputStrength;
      }

      dominator = dominator == 0? 1 : dominator;

      return (MembershipType) (numerator / dominator);
    }

  private:

    // Two hash tables are used to store input and output membership functions
    __gnu_cxx::hash_map <const char*, MembershipFunction<MembershipType> >
      inputMembershipFunctions,
      outputMembershipFunctions;
    // The following iterator is used to iterate on the previous hashtables
    typename __gnu_cxx::hash_map <const char*,
                                  MembershipFunction<MembershipType> >::iterator
      membershipFunctionsIterator;
    // The following vector is used to store rules
    std::vector<Rule<MembershipType> > rules;
    /*
     * The following variable is used to store the count of rules instead of
     *   calculating it everytime.
     * */
    std::size_t rulesCount;
    // The following are temp. vectors used during the construction
    //   of a new rule.
    std::vector<MembershipFunction<MembershipType>*> antecedents,
                                                     consequents;
};

}

#endif // #ifndef UTILITIES_FUZZYCONTROLLER_H
