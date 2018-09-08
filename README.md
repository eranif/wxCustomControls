# clTreeCtrl

This control has a consistent look and feel across all platforms. It uses the exact `wxTreeCtrl` API with slight modifications. The goal is to be able to use it as a drop in replacement in your code for `wxTreeCtrl`.
In addition to the traditional tree view, `clTreeCtrl` allows you to display columns (similar to `wxTreeListCtrl`)

Advantages over the standrd wx controls out there:
- Performance & Optimization: you can display thousands (and more, much more) entries, and the performance is close to instant
- Customisation: you can colour any part of the control (text, selected item text, font, selected text, selected text background, hover bg colour, expand/collapse button, unlike other controls, you can do this per cell. 
- scrollbar on demand: the scrollbar to can be set hide when the control loses the focus
- Sorting: one of the head aches using `wxTreeCtrl`, is sorting items. `clTreeCtrl`, uses modern C++ API to support this. 

For example, lets assume you have a tree representing the file system. You want folders to be displayed first, sorted A-Z, followed by files (which should also be sorted A-Z)

Also, lets assume that you keep client data per node, holding the node type, so your code should look something like this:

```c++
clTreeCtrl* tree = new clTreeCtrl(this);
/// The function returns true if A should placed BEFORE B
std::function<bool(const wxTreeItemId&, const wxTreeItemId&)> SortFunc = [&](const wxTreeItemId& itemA,
                                                                             const wxTreeItemId& itemB) {
    clTreeCtrlData* a = dynamic_cast<clTreeCtrlData*>(GetItemData(itemA));
    clTreeCtrlData* b = dynamic_cast<clTreeCtrlData*>(GetItemData(itemB));
    if(a->IsFolder() && b->IsFile())
        return true;
    else if(b->IsFolder() && a->IsFile())
        return false;
    // same kind
    return (a->GetName().CmpNoCase(b->GetName()) < 0);
};
tree->SetSortFunction(SortFunc );
```

Table view:

Continuing with the above example, assuming you want a table with 3 columns: `path`, `type` and `size`, the code should look similar to  this:

```c++
    clHeaderBar header;
    header.Add("Path");
    header.Add("Kind");
    header.Add("Size");
    m_tree->SetHeader(header);
```

Adding row (with 3 columns):

```c++              
// Add the item (this will fill column 0 content)
wxTreeItemId folderItem = m_tree->AppendItem(item, filename, 0, 1, new MyItemData(fn.GetFullPath(), true));
// Add data in columns 1 and 2
m_tree->SetItemText(folderItem, "Folder", 1);
m_tree->SetItemText(folderItem, "0KB", 2);
```

As a bonus, no more `wxImageList`! - `clTreeCtrl` uses `std::vector<wxBitmap>` for storing the bitmaps.

The code is licensed under the MIT license, see the LICENSE file for more details

![alt text](https://github.com/eranif/clTreeCtrl/blob/master/clTreeCtrl-Linux.png)

![alt text](https://github.com/eranif/clTreeCtrl/blob/master/clTreeCtrl-Windows.png)
