/* tablesort.h - sortable columns for the interface's tables: a click on a column's header sorts
 * the rows by it (again: the other way round; Shift-click: by several). Call table_order right
 * after TableHeadersRow of a table made with ImGuiTableFlags_Sortable; it returns the rows in the
 * order to draw them. The order is kept until a header is clicked or the rows change (a filter),
 * so a row being edited does not jump away while it is typed in. */
#pragma once
#include "imgui.h"
#include <vector>
#include <string>
#include <algorithm>
#include <strings.h>

struct SortKey {
    double n = 0; std::string s; bool str = false;
    SortKey(double v) : n(v) {}
    SortKey(int v) : n(v) {}
    SortKey(const std::string &v) : s(v), str(true) {}
    SortKey(const char *v) : s(v ? v : ""), str(true) {}
};

struct TableSort { std::vector<int> rows, order; };

/* rows: the row ids (in their natural order); key(row, column) the value to sort that cell by */
template <class F> const std::vector<int> &table_order(TableSort &ts, const std::vector<int> &rows, F key)
{
    ImGuiTableSortSpecs *sp = ImGui::TableGetSortSpecs();
    std::vector<int> &cache = ts.order;
    if ((sp && sp->SpecsDirty) || ts.rows != rows) {
        ts.rows = rows;
        cache = rows;
        if (sp && sp->SpecsCount > 0)
            std::stable_sort(cache.begin(), cache.end(), [&](int a, int b) {
                for (int k = 0; k < sp->SpecsCount; k++) {
                    const ImGuiTableColumnSortSpecs &c = sp->Specs[k];
                    SortKey x = key(a, c.ColumnIndex), y = key(b, c.ColumnIndex);
                    int d = x.str || y.str ? strcasecmp(x.s.c_str(), y.s.c_str()) : (x.n < y.n ? -1 : x.n > y.n ? 1 : 0);
                    if (d) return c.SortDirection == ImGuiSortDirection_Descending ? d > 0 : d < 0;
                }
                return false;
            });
        if (sp) sp->SpecsDirty = false;
    }
    return cache;
}
