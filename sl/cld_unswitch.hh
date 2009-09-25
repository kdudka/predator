#ifndef H_GUARD_CLD_UNSWITCH_H
#define H_GUARD_CLD_UNSWITCH_H

struct ICodeListener;

/**
 * create ICgListener implementation (TODO: document)
 * @return on heap allocated instance of ICgListener object
 */
ICodeListener* createCldUnfoldSwitch(ICodeListener *);

#endif /* H_GUARD_CLD_UNSWITCH_H */
