#ifndef BOX_H
#define BOX_H

#include <vector>
#include <set>
#include <string>
#include <stdexcept>
#include <fstream>

#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
//#include "forestaut.hh"

#include "cache.hh"
#include "varinfo.hh"
#include "labman.hh"
#include "forestaut.hh"
#include "tatimint.hh"

using std::vector;
using std::string;
using std::pair;

class BoxTemplate {

	friend class TemplateManager;
	friend class Box;

	size_t type;
	size_t tag;

	typedef TA<const vector<const BoxTemplate*>*> root_type;

	vector<var_info> variables;
	vector<root_type> roots;

protected:
/*
	static FA::label_type translateLabel(LabMan& labMan, const vector<const BoxTemplate*>* label, const boost::unordered_map<const BoxTemplate*, const Box*>& args) {
		vector<const Box*> v;
		for (vector<const BoxTemplate*>::const_iterator i = label->begin(); i != label->end(); ++i) {
			boost::unordered_map<const BoxTemplate*, const Box*>::const_iterator j = args.find(*i);
			if (j == args.end())
				throw std::runtime_error("template instance undefined");
			v.push_back(j->second);
		}
		return &labMan.lookup(v);
	}
*/
	BoxTemplate(size_t type, size_t tag) : type(type), tag(tag) {}

public:

	static BoxTemplate createBoxTemplate() {
		return BoxTemplate(0, 0);
	}

	static BoxTemplate createSelectorTemplate(size_t selector) {
		return BoxTemplate(1, selector);
	}

	static BoxTemplate createReferenceTemplate(size_t root) {
		return BoxTemplate(2, root);
	}
/*
	Box createInstance(TAManager<FA::label_type>& taMan, LabMan& labMan, const vector<size_t>& offsets, const boost::unordered_map<const BoxTemplate*, const Box*>& args) const {
		Box box(taMan, *this);
		if (this->type == 1) {
			assert(offsets.size() == 2);
			box.variables.push_back(var_info(0, offsets[0]));
			box.variables.push_back(var_info(1, offsets[1]));
			return box;
		}
		if (this->type == 2)
			return box;
		assert(offsets.size() == this->variables.size());
		for (vector<root_type>::const_iterator j = this->roots.begin(); j != this->roots.end(); ++j) {
			TA<FA::label_type>* ta = taMan.alloc();
			for (root_type::iterator i = j->begin(); i != j->end(); ++i)
				ta->addTransition(i->lhs(), BoxTemplate::translateLabel(labMan, i->label(), args), i->rhs());
			ta->addFinalState(j->getFinalState());
			box.roots.push_back(ta);
		}
		for (size_t i = 0; i < this->variables.size(); ++i)
			box.variables.push_back(var_info(this->variables[i].index, offsets[i]));
	}
*/
	void computeTrigger(vector<const BoxTemplate*>& templates) {
		templates.clear();
		std::set<const BoxTemplate*> s;
		for (root_type::iterator i = this->roots[0].begin(); i != this->roots[0].end(); ++i) {
			if (this->roots[0].isFinalState(i->rhs()))
				s.insert(i->label()->begin(), i->label()->end());
		}
		templates = vector<const BoxTemplate*>(s.begin(), s.end());
	}

	bool isSelector() const {
		return this->type == 1;
	}

	bool isSelector(size_t which) const {
		return this->type == 1 && this->tag == which;
	}

	size_t getSelector() const {
		return this->tag;
	}

	bool isReference() const {
		return this->type == 2;
	}

	bool isReference(size_t which) const {
		return this->type == 2 && this->tag == which;
	}

	size_t getReference() const {
		return this->tag;
	}

};

class Box : public FA {

//	friend class BoxTemplate;

	const BoxTemplate& templ;

protected:

	static FA::label_type translateLabel(LabMan& labMan, const vector<const BoxTemplate*>* label, const boost::unordered_map<const BoxTemplate*, const Box*>& args) {
		vector<const Box*> v;
		for (vector<const BoxTemplate*>::const_iterator i = label->begin(); i != label->end(); ++i) {
			boost::unordered_map<const BoxTemplate*, const Box*>::const_iterator j = args.find(*i);
			if (j == args.end())
				throw std::runtime_error("template instance undefined");
			v.push_back(j->second);
		}
		return &labMan.lookup(v);
	}

public:

	Box(const BoxTemplate& templ, TAManager<FA::label_type>& taMan, LabMan& labMan, const vector<size_t>& offsets, const boost::unordered_map<const BoxTemplate*, const Box*>& args)
		: FA(taMan), templ(templ) {
		if (this->templ.type == 1) {
			assert(offsets.size() == 2);
			this->variables.push_back(var_info(0, offsets[0]));
			this->variables.push_back(var_info(1, offsets[1]));
		} else if (this->templ.type == 0) {
			assert(offsets.size() == this->templ.variables.size());
			for (vector<BoxTemplate::root_type>::const_iterator j = this->templ.roots.begin(); j != this->templ.roots.end(); ++j) {
				TA<FA::label_type>* ta = taMan.alloc();
				for (BoxTemplate::root_type::iterator i = j->begin(); i != j->end(); ++i)
					ta->addTransition(i->lhs(), Box::translateLabel(labMan, i->label(), args), i->rhs());
				ta->addFinalState(j->getFinalState());
				this->roots.push_back(ta);
			}
			for (size_t i = 0; i < this->templ.variables.size(); ++i)
				this->variables.push_back(var_info(this->templ.variables[i].index, offsets[i]));
		}
	}
/*
	Box(TAManager<label_type>& taMan, const class BoxTemplate& templ) : FA(taMan), templ(templ) {
		
	}
*/
	const class BoxTemplate& getTemplate() const {
		return this->templ;
	}
/*
	bool isPrimitive() const {
		return this->type == 0;
	}

	bool isRootReference() const {
		return this->type == 1;
	}
*/
	


	friend size_t hash_value(const Box& b) {
		return hash_value(hash_value(&b.templ) + hash_value(b.variables));
	}

	bool operator==(const Box& rhs) const {
		return (&this->templ == &rhs.templ) && (this->variables == rhs.variables);
	}

};

class TemplateManager {

	Cache<std::vector<const class BoxTemplate*> > templateStore;

	TA<const vector<const BoxTemplate*>*>::Backend backend;

	mutable LabMan& labMan;

	boost::unordered_map<string, BoxTemplate> templates;

public:

	static const char* selectorPrefix;
	static const char* referencePrefix;
	static const char* nullStr;

	static bool isSelector(const std::string& name) {
		return memcmp(name.c_str(), selectorPrefix, strlen(selectorPrefix)) == 0;
	}

	static size_t getSelector(const std::string& name) {
		assert(TemplateManager::isSelector(name));
		return atol(name.c_str() + strlen(selectorPrefix));
	}

	static bool isReference(const std::string& name) {
		return (name == nullStr) || (memcmp(name.c_str(), referencePrefix, strlen(referencePrefix)) == 0);
	}

	static size_t getReference(const std::string& name) {
		assert(TemplateManager::isReference(name));
		if (name == nullStr)
			return FA::varNull;
		return atol(name.c_str() + strlen(referencePrefix));
	}

protected:

	BoxTemplate& loadTemplate(const string& name, const boost::unordered_map<string, string>& database) {

		boost::unordered_map<string, BoxTemplate>::iterator i = this->templates.find(name);
		if (i != this->templates.end())
			return i->second;

		if (TemplateManager::isSelector(name))
			return this->templates.insert(
				make_pair(name, BoxTemplate::createSelectorTemplate(TemplateManager::getSelector(name)))
			).first->second;

		if (TemplateManager::isReference(name))
			return this->templates.insert(
				make_pair(name, BoxTemplate::createReferenceTemplate(TemplateManager::getReference(name)))
			).first->second;

		boost::unordered_map<string, string>::const_iterator j = database.find(name);
		if (j == database.end())
			throw std::runtime_error("Template '" + name + "' not found!");

		BoxTemplate& templ = this->templates.insert(
			make_pair(name, BoxTemplate::createBoxTemplate())
		).first->second;

		std::fstream input(j->second);

		TAReader reader(input, j->second);

		TA<string>::Backend sBackend;
		TA<string> sta(sBackend);

		TA<const vector<const BoxTemplate*>*> ta(this->backend);

		string autName;

		reader.readFirst(sta, autName);

		this->translateTemplate(ta, sta, database);
		templ.variables.push_back(var_info(templ.roots.size(), 0));
		templ.roots.push_back(ta);

		while (reader.readNext(sta, autName)) {
			ta.clear();
			this->translateTemplate(ta, sta, database);
			if (memcmp(autName.c_str(), "in", 2) == 0)
				templ.variables.push_back(var_info(templ.roots.size(), 0));
			templ.roots.push_back(ta);
		}

		return templ;

	}

	void translateTemplate(TA<const vector<const BoxTemplate*>*>& dst, const TA<string>& src, const boost::unordered_map<string, string>& database) {
		dst.clear();
		for (TA<string>::iterator i = src.begin(); i != src.end(); ++i) {
			vector<string> strs;
			boost::split(strs, i->label(), boost::is_from_range('_', '_'));
			vector<const BoxTemplate*> label;
			for (vector<string>::iterator j = strs.begin(); j != strs.end(); ++j)
				label.push_back(&TemplateManager::loadTemplate(*j, database));
			dst.addTransition(i->lhs(), &this->templateStore.lookup(label)->first, i->rhs());
		}
		dst.addFinalState(src.getFinalState());
	}

public:

	TemplateManager(LabMan& labMan) : labMan(labMan) {}

	void loadTemplates(const boost::unordered_map<string, string>& database) {
		for (boost::unordered_map<string, string>::const_iterator i = database.begin(); i != database.end(); ++i)
			this->loadTemplate(i->first, database);
	}

	LabMan& getLabMan() const {
		return this->labMan;
	}

};

#endif
