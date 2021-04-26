#ifndef MODEL_ADAPTER_H
#define MODEL_ADAPTER_H

#include "mp/convert/converter_query.h"

namespace mp {

/// Present an interface to "original" model
/// corresponding to the NL file
/// This is a workaround to use MP output facilities
template <class Model>
class ModelAdapter : public Model {
  int n_vars_orig_ = -1;
  int n_alg_cons_orig_= -1;
public:
  const Model& GetModel() const { return *this; }
  Model& GetModel() { return *this; }

  /// MP solution writing uses the following model size queries
  /** Returns the number of variables. */
  int num_vars() const { return n_vars_orig_; }
  /** Returns the number of algebraic constraints. */
  int num_algebraic_cons() const { return n_alg_cons_orig_; }
  void set_num_vars(int n) { n_vars_orig_ = n; }
  void set_num_alg_cons(int n) { n_alg_cons_orig_ = n; }


  ////////////////////////// SUFFIX DECLARE & OUTPUT ///////////////////////
  void DeclareAndReportIntSuffix(const SuffixDef<int>& suf,
                                 const std::vector<int>& values) {
    DeclareAndReportSuffix(suf, values);
  }
  void DeclareAndReportDblSuffix(const SuffixDef<double>& suf,
                                 const std::vector<double>& values) {
    DeclareAndReportSuffix(suf, values);
  }

  template <class T>
  void DeclareAndReportSuffix(const SuffixDef<int>& sufdef,
                              const std::vector<T>& values) {
    if (values.empty())
      return;
    auto main_kind = (suf::Kind)(sufdef.kind() & suf::KIND_MASK);
    auto suf_raw = Model::suffixes(main_kind).Find(sufdef.name());
    auto suf_size = GetSuffixSize(main_kind);    // can be < values.size()
    assert(suf_size <= (int)values.size());
    auto suf =
      (suf_raw) ? (typename Model::SuffixHandler<T>)
               Cast<BasicMutSuffix<T> >( suf_raw )
        : (typename Model::SuffixHandler<T>)
          Model::suffixes(main_kind).template
          Add<T>(sufdef.name(), sufdef.kind(), suf_size);
    for (auto i=suf_size; i--; ) {
      suf.SetValue(i, values[i]);
    }
  }

  int GetSuffixSize(suf::Kind kind) {
    std::size_t size = 0;
    switch (kind) {
    default:
      MP_ASSERT(false, "invalid suffix kind");
      // Fall through.
    case suf::VAR:
      size = num_vars();
      break;
    case suf::CON:
      size = num_algebraic_cons();
      break;
    case suf::OBJ:
      size = Model::num_objs();
      break;
    case suf::PROBLEM:
      size = 1;
      break;
    }
    return static_cast<int>(size);
  }


};

} // namespace mp

#endif // MODEL_ADAPTER_H
