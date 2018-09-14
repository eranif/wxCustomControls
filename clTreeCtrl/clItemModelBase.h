#ifndef CLITEMMODELBASE_H
#define CLITEMMODELBASE_H

#include "codelite_exports.h"

class clRowEntry;
class WXDLLIMPEXP_SDK clItemModelBase
{
public:
    clItemModelBase();
    virtual ~clItemModelBase();
    
    virtual void NodeDeleted(clRowEntry* item) = 0;
    /**
     * @brief can a node expand/collapse?
     * @param item row item
     * @param expanding true when expanding, false for collapsing
     */
    virtual bool NodeExpanding(clRowEntry* item, bool expanding) = 0;
    /**
     * @brief Node expanded / collapsed
     * @param item row item
     * @param expanded true when expanded, false for collapsed
     */
    virtual void NodeExpanded(clRowEntry* item, bool expanded) = 0;
};

#endif // CLITEMMODELBASE_H
