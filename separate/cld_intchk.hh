#ifndef H_GUARD_CLD_INTCHK_H
#define H_GUARD_CLD_INTCHK_H

struct ICodeListener;

/**
 * create ICgListener implementation (TODO: document)
 * @return on heap allocated instance of ICgListener object
 */
ICodeListener* createCldIntegrityChk(ICodeListener *);

#endif /* H_GUARD_CLD_INTCHK_H */
