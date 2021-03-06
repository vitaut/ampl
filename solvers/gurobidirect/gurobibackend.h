#ifndef MP_GUROBI_BACKEND_H_
#define MP_GUROBI_BACKEND_H_

#if __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunused-parameter"
# pragma clang diagnostic ignored "-Wunused-private-field"
#elif _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4244)
#endif

extern "C" {
  #include "gurobi_c.h"
}

#if __clang__
# pragma clang diagnostic pop
#elif _MSC_VER
# pragma warning(pop)
#endif

#include <string>

#include "mp/convert/MIP/backend.h"
#include "mp/convert/std_constr.h"

namespace mp {

class GurobiBackend : public MIPBackend<GurobiBackend>
{
  using BaseBackend = MIPBackend<GurobiBackend>;

  //////////////////// [[ The public interface ]] //////////////////////
public:
  GurobiBackend();
  ~GurobiBackend();

  /// Metadata
  static const char* GetSolverName() { return "Gurobi"; }
  static std::string GetSolverVersion();
  static const char* GetSolverInvocationName();
  static const char* GetAMPLSolverLongName() { return nullptr; }
  static const char* GetBackendName();
  static const char* GetBackendLongName() { return nullptr; }

  /// Solver flags
  static bool IfMultipleSol() { return true; }
  static bool IfMultipleObj() { return true; }

  /// [[ Prototype an incremental interface ]]
  void InitProblemModificationPhase();
  void FinishProblemModificationPhase();

  void AddVariable(Variable var);
  void AddLinearObjective( const LinearObjective& lo );
  void AddQuadraticObjective( const QuadraticObjective& qo );

  //////////////////////////// GENERAL CONSTRAINTS ////////////////////////////
  USE_BASE_CONSTRAINT_HANDLERS(BaseBackend)

  /// TODO Attributes (lazy/user cut, etc)
  ACCEPT_CONSTRAINT(LinearConstraint, Recommended)
  void AddConstraint(const LinearConstraint& lc);
  ACCEPT_CONSTRAINT(QuadraticConstraint, Recommended)
  void AddConstraint(const QuadraticConstraint& qc);
  ACCEPT_CONSTRAINT(MaximumConstraint, AcceptedButNotRecommended)
  void AddConstraint(const MaximumConstraint& mc);
  ACCEPT_CONSTRAINT(MinimumConstraint, AcceptedButNotRecommended)
  void AddConstraint(const MinimumConstraint& mc);
  ACCEPT_CONSTRAINT(AbsConstraint, AcceptedButNotRecommended)
  void AddConstraint(const AbsConstraint& absc);
  ACCEPT_CONSTRAINT(ConjunctionConstraint, AcceptedButNotRecommended)
  void AddConstraint(const ConjunctionConstraint& cc);
  ACCEPT_CONSTRAINT(DisjunctionConstraint, AcceptedButNotRecommended)
  void AddConstraint(const DisjunctionConstraint& mc);
  /// Enabling built-in indicator for infinite bounds,
  /// but not recommended otherwise --- may be slow
  ACCEPT_CONSTRAINT(IndicatorConstraintLinLE, AcceptedButNotRecommended)
  void AddConstraint(const IndicatorConstraintLinLE& mc);

  /// Nonlinear
  ACCEPT_CONSTRAINT(ExpConstraint, Recommended)
  void AddConstraint(const ExpConstraint& cc);
  ACCEPT_CONSTRAINT(ExpAConstraint, Recommended)
  void AddConstraint(const ExpAConstraint& cc);
  ACCEPT_CONSTRAINT(LogConstraint, Recommended)
  void AddConstraint(const LogConstraint& cc);
  ACCEPT_CONSTRAINT(LogAConstraint, Recommended)
  void AddConstraint(const LogAConstraint& cc);
  ACCEPT_CONSTRAINT(PowConstraint, Recommended)
  void AddConstraint(const PowConstraint& cc);
  ACCEPT_CONSTRAINT(SinConstraint, Recommended)
  void AddConstraint(const SinConstraint& cc);
  ACCEPT_CONSTRAINT(CosConstraint, Recommended)
  void AddConstraint(const CosConstraint& cc);
  ACCEPT_CONSTRAINT(TanConstraint, Recommended)
  void AddConstraint(const TanConstraint& cc);


  ///////////////////// Model attributes /////////////////////
  bool IsMIP() const;
  bool IsQCP() const;

  int NumberOfConstraints() const;
  int NumberOfVariables() const;
  int NumberOfObjectives() const;

  void ExportModel(const std::string& file);


  //////////////////////////// SOLVING ///////////////////////////////
  void SetInterrupter(mp::Interrupter* inter);
  void SolveAndReportIntermediateResults();
  std::string ConvertSolutionStatus(
      const mp::Interrupter &interrupter, int &solve_code);

  /// Various solution attribute getters.
  /// Return empty vectors if not available
  std::vector<double> PrimalSolution();
  std::vector<double> DualSolution();
  double ObjectiveValue() const;

  double NodeCount() const;
  double Niterations() const;

  std::vector<int> VarStatii();
  std::vector<int> ConStatii();

  /// Various attribute setters
  void VarStatii(ArrayRef<int> );
  void ConStatii(ArrayRef<int> );


  ////////////////////////////////////////////////////////////
  /////////////// OPTIONAL STANDARD FEATURES /////////////////
  ////////////////////////////////////////////////////////////

  /**
  * Compute the IIS and obtain relevant values
  **/
  ALLOW_STD_FEATURE( IIS, true )
  void ComputeIIS();
  /// Elements correspond to IISStatus
  std::vector<int> VarsIIS();
  std::vector<int> ConsIIS();
  /**
  * Get MIP Gap. +inf if not available
  **/
  ALLOW_STD_FEATURE( MIPGap, true )
  double MIPGap() const;
  /**
  * Set branch and bound priority
  **/
  ALLOW_STD_FEATURE( VarPriority, true )
  void VarPriority(ArrayRef<int> );

  /// Option "multiobj" is created internally whenever
  /// IfMultipleObj() return true. Can change the help text here
  const char* GetMultiobjHelpText() const override;

  //////////////////// [[ Implementation details ]] //////////////////////
  ///////////////////////////////////////////////////////////////////////////////
private:
  GRBenv   *env   = NULL;
  GRBmodel *model = NULL;
  
   // Stores gurobi optimization status after SolveAndReportIntermediateResults
  int optimstatus;

public:
  void OpenSolver();
  void CloseSolver();
  void InitCustomOptions();

  static double Infinity() { return GRB_INFINITY; }
  static double MinusInfinity() { return -GRB_INFINITY; }

  /// if (flag), set *flag <-> success,
  /// otherwise fail on error
  int GetGrbIntAttribute(const char* attr_id, bool *flag=nullptr) const;
  double GetGrbDblAttribute(const char* attr_id, bool *flag=nullptr) const;

  /// Return empty vector on failure
  std::vector<int> GetGrbIntArrayAttribute(const char* attr_id,
    std::size_t size, std::size_t offset=0) const;
  std::vector<double> GetGrbDblArrayAttribute(const char* attr_id,
    std::size_t size, std::size_t offset=0) const;

  /// Return false on failure
  bool SetGrbIntArrayAttribute(const char* attr_id,
                               ArrayRef<int> values, std::size_t start=0);
  bool SetGrbDblArrayAttribute(const char* attr_id,
                               ArrayRef<double> values, std::size_t start=0);

private:
  /// These options are stored in the class as variables
  /// for direct access
  struct Options {
    std::string exportFile_;
  };

  Options storedOptions_;


public:
  /// These methods access Gurobi options. Used by AddSolverOption()
  void GetSolverOption(const char* key, int& value) const;
  void SetSolverOption(const char* key, int value);
  void GetSolverOption(const char* key, double& value) const;
  void SetSolverOption(const char* key, double value);
  void GetSolverOption(const char* key, std::string& value) const;
  void SetSolverOption(const char* key, const std::string& value);

};

} // namespace mp

#endif  // MP_GUROBI_BACKEND_H_
