#include "cld_unilabel.hh"
#include "cl_decorator.hh"

#include <map>
#include <sstream>
#include <string>

class CldUniLabel: public ClDecoratorBase {
    public:
        CldUniLabel(ICodeListener *slave, cl_scope_e scope);

        virtual void file_open(const char *file_name) {
            if (CL_SCOPE_STATIC == scope_)
                this->reset();
            ClDecoratorBase::file_open(file_name);
        }

        virtual void fnc_open(
            struct cl_location      *loc,
            const char              *fnc_name,
            enum cl_scope_e         scope)
        {
            if (CL_SCOPE_FUNCTION == scope_)
                this->reset();
            ClDecoratorBase::fnc_open(loc, fnc_name, scope);
        }

        virtual void bb_open(
            const char              *bb_name)
        {
            std::string resolved(this->resolveLabel(bb_name));
            ClDecoratorBase::bb_open(resolved.c_str());
        }


        virtual void insn_jmp(
            struct cl_location      *loc,
            const char              *label)
        {
            std::string resolved(this->resolveLabel(label));
            ClDecoratorBase::insn_jmp(loc, resolved.c_str());
        }

        virtual void insn_cond(
            struct cl_location      *loc,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false)
        {
            std::string resolved1(this->resolveLabel(label_true));
            std::string resolved2(this->resolveLabel(label_false));
            ClDecoratorBase::insn_cond(loc, src, resolved1.c_str(),
                                   resolved2.c_str());
        }

    private:
        typedef std::map<std::string, int> TMap;

        cl_scope_e      scope_;
        TMap            map_;
        int             last_;

    private:
        std::string resolveLabel(const char *);
        int labelLookup(const char *);
        void reset();
};

CldUniLabel::CldUniLabel(ICodeListener *slave, cl_scope_e scope):
    ClDecoratorBase(slave),
    scope_(scope),
    last_(0)
{
    switch (scope) {
        case CL_SCOPE_GLOBAL:
        case CL_SCOPE_STATIC:
        case CL_SCOPE_FUNCTION:
            break;

        default:
            CL_DIE("invalid scope for CldUniLabel decorator");
    }
}

std::string CldUniLabel::resolveLabel(const char *label) {
    std::ostringstream str;
    str << "L" << this->labelLookup(label);
    return str.str();
}

int CldUniLabel::labelLookup(const char *label) {
    std::string str(label);

    TMap::iterator i = map_.find(str);
    if (map_.end() != i)
        return i->second;

    map_[str] = ++last_;
    return last_;
}

void CldUniLabel::reset() {
    map_.clear();
    last_ = 0;
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cld_unilabel.hh for more details
ICodeListener* createCldUniLabel(ICodeListener *slave, cl_scope_e scope) {
    return new CldUniLabel(slave, scope);
}
