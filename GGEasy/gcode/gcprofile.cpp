// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gcprofile.h"
#include "gcfile.h"

#include "bridgeitem.h"

#include "scene.h"

#include "leakdetector.h"

namespace GCode {
ProfileCreator::ProfileCreator()
{
}

void ProfileCreator::create()
{
    createProfile(m_gcp.tools.front(), m_gcp.params[GCodeParams::Depth].toDouble());
}

GCodeType ProfileCreator::type() { return Profile; }

void ProfileCreator::createProfile(const Tool& tool, const double depth)
{

    m_toolDiameter = tool.getDiameter(depth);

    const double dOffset = (m_gcp.side() == Outer) ? +m_toolDiameter * uScale * 0.5 : -m_toolDiameter * uScale * 0.5;

    if (m_gcp.side() == On) {
        if (m_gcp.params[GCodeParams::Strip].toBool())
            trimmingOpenPaths(m_workingRawPs);

        m_returnPs = m_workingPs;

        for (Path& path : m_returnPs)
            path.push_back(path.front());

    } else {
        if (!m_workingPs.empty()) {
            ClipperOffset offset;
            for (Paths& paths : groupedPaths(CopperPaths))
                offset.AddPaths(paths, jtRound, etClosedPolygon);
            offset.Execute(m_returnPs, dOffset);
        }

        if (!m_workingRawPs.empty()) {
            ClipperOffset offset;
            offset.AddPaths(m_workingRawPs, jtRound, etOpenRound);
            offset.Execute(m_workingRawPs, dOffset);
            if (!m_workingRawPs.empty())
                m_returnPs.append(m_workingRawPs);
        }
    }

    if (m_returnPs.empty()) {
        emit fileReady(nullptr);
        return;
    }

    reorder();

    if (m_gcp.side() == On && m_workingRawPs.size()) {
        m_returnPss.reserve(m_returnPss.size() + m_workingRawPs.size());
        for (auto&& path : m_workingRawPs)
            m_returnPss.push_back({ std::move(path) });
    }

    // find Bridges
    mvector<BridgeItem*> bridgeItems;
    for (QGraphicsItem* item : App::scene()->items()) {
        if (static_cast<GiType>(item->type()) == GiType::Bridge)
            bridgeItems.push_back(static_cast<BridgeItem*>(item));
    }
    // create Bridges
    if (bridgeItems.size()) {
        for (auto& m_returnPs : m_returnPss) {
            const Path& path = m_returnPs.front();
            std::vector<std::pair<BridgeItem*, IntPoint>> biStack;
            biStack.reserve(bridgeItems.size());
            IntPoint pt;
            for (BridgeItem* bi : bridgeItems) {
                if (pointOnPolygon(bi->getPath(), path, &pt))
                    biStack.emplace_back(bi, pt);
            }
            if (!biStack.empty()) {
                Paths paths;
                // create frame
                {
                    ClipperOffset offset;
                    offset.AddPath(path, jtMiter, etClosedLine);
                    offset.Execute(paths, +m_toolDiameter * uScale * 0.1);

                    Clipper clipper;
                    clipper.AddPaths(paths, ptSubject, true);
                    for (const QPair<BridgeItem*, IntPoint>& bip : biStack) {
                        clipper.AddPath(CirclePath((bip.first->lenght() + m_toolDiameter) * uScale, bip.second), ptClip, true);
                    }
                    clipper.Execute(ctIntersection, paths, pftPositive);
                }
                // cut toolPath
                {
                    Clipper clipper;
                    clipper.AddPath(path, ptSubject, false);
                    clipper.AddPaths(paths, ptClip, true);
                    PolyTree polytree;
                    clipper.Execute(ctDifference, polytree, pftPositive);
                    PolyTreeToPaths(polytree, paths);
                }
                // merge result toolPaths
                mergeSegments(paths);
                sortBE(paths);

                auto check = [&paths, &path] {
                    for (size_t i = 0, end = path.size() - 1; i < end; ++i) {
                        auto srcPt(path[i]);
                        for (const auto& rPath : paths) {
                            for (size_t j = 0, rEnd = rPath.size() - 1; j < rEnd; ++j) {
                                if (srcPt == rPath[j]) {
                                    if (path[i + 1] == rPath[j + 1]) {
                                        return false;
                                    } else if (j && path[i + 1] == rPath[j - 1]) {
                                        return true;
                                    } else if (j && i && path[i - 1] == rPath[j - 1]) {
                                        return false;
                                    }
                                }
                            }
                        }
                    }
                    return false; // ???
                };

                if (check())
                    ReversePaths(paths);

                std::swap(m_returnPs, paths);
            }
        }
    }

    if (m_returnPss.empty()) {
        emit fileReady(nullptr);
    } else {
        m_gcp.gcType = Profile;
        m_file = new File(m_returnPss, m_gcp);
        m_file->setFileName(tool.nameEnc());
        emit fileReady(m_file);
    }
}

void ProfileCreator::trimmingOpenPaths(Paths& paths)
{
    const double dOffset = m_toolDiameter * uScale * 0.5;
    for (size_t i = 0; i < paths.size(); ++i) {
        auto& p = paths[i];
        if (p.size() == 2) {
            double l = p.front().distTo(p.back());
            if (l <= m_toolDiameter * uScale) {
                paths.remove(i--);
                continue;
            }
            QLineF b(p.front(), p.back());
            QLineF e(p.back(), p.front());
            b.setLength(b.length() - m_toolDiameter * 0.5);
            e.setLength(e.length() - m_toolDiameter * 0.5);
            p = { (b.p2()), (e.p2()) };
        } else if (double l = Perimeter(p); l <= m_toolDiameter * uScale) {
            paths.remove(i--);
            continue;
        } else {
            Paths ps;
            {
                ClipperOffset offset;
                offset.AddPath(p, jtMiter, etOpenButt);
                offset.Execute(ps, dOffset + 100);
                //dbgPaths(ps, {}, "offset+");
                offset.Clear();
                offset.AddPath(ps.front(), jtMiter, etClosedPolygon);
                offset.Execute(ps, -dOffset);
                //dbgPaths(ps, {}, "offset-");
                if (ps.empty()) {
                    paths.remove(i--);
                    continue;
                }
            }
            {
                Clipper clipper;
                clipper.AddPath(p, ptSubject, false);
                clipper.AddPaths(ps, ptClip, true);
                clipper.Execute(ctIntersection, ps, pftPositive);
                //dbgPaths(ps, {}, "clip-");
                p = ps.front();
            }
        }
    }
}

void ProfileCreator::reorder()
{
    PolyTree polyTree;
    {
        Clipper clipper;
        clipper.AddPaths(m_returnPs, ptSubject);
        IntRect r(clipper.GetBounds());
        int k = uScale;
        Path outer = {
            { r.left - k, r.bottom + k },
            { r.right + k, r.bottom + k },
            { r.right + k, r.top - k },
            { r.left - k, r.top - k }
        };
        clipper.AddPath(outer, ptSubject, true);
        clipper.Execute(ctUnion, polyTree, pftEvenOdd);
        m_returnPs.clear();
    }

    polyTreeToPaths(polyTree, m_returnPs);
    //    m_returnPs.takeFirst();

    std::reverse(m_returnPs.begin(), m_returnPs.end());

    if ((m_gcp.side() == Inner) ^ m_gcp.convent())
        ReversePaths(m_returnPs);

    //    int ctr1 = 0, ctr2 = 0, j = 0, k = 0;
    //    for (size_t i = 1; i < m_returnPs.size(); ++i) {
    //        Path& path1 = m_returnPs[i - 1];
    //        Path& path2 = m_returnPs[i];
    //        if (i == 1) {
    //            double d = std::numeric_limits<double>::max();
    //            ctr1 = 0;
    //            for (auto pt1 : path1) {
    //                ctr2 = 0;
    //                for (auto pt2 : path2) {
    //                    if (auto tmp = pt1.distTo(pt2); d > tmp) {
    //                        d = tmp;
    //                        j = ctr1;
    //                        k = ctr2;
    //                    }
    //                    ++ctr2;
    //                }
    //                ++ctr1;
    //            }
    //            std::rotate(path1.begin(), path1.begin() + j, path1.end());
    //            std::rotate(path2.begin(), path2.begin() + k, path2.end());
    //        } else {
    //            double d = std::numeric_limits<double>::max();
    //            auto pt1 = path1.front();
    //            ctr2 = 0;
    //            for (auto pt2 : path2) {
    //                if (auto tmp = pt1.distTo(pt2); d > tmp) {
    //                    d = tmp;
    //                    k = ctr2;
    //                }
    //                ++ctr2;
    //            }
    //            std::rotate(path2.begin(), path2.begin() + k, path2.end());
    //        }
    //    }

    m_returnPss.reserve(m_returnPs.size());
    //    for (size_t i = 0; i < m_returnPs.size(); ++i) {
    //        m_returnPs[i].push_back(m_returnPs[i].front());
    //        m_returnPss[i].emplace_back(std::move(m_returnPs[i]));
    //    }
    for (auto&& path : m_returnPs) {
        path.push_back(path.front());
        m_returnPss.push_back({ path });
    }
}

//void ProfileCreator::addPolyNodeToPaths(PolyNode& polynode, ProfileCreator::NodeType nodetype, Paths& paths)
//{
//    bool match = true;
//    if (nodetype == ntClosed)
//        match = !polynode.IsOpen();
//    else if (nodetype == ntOpen)
//        return;

//    if (!polynode.Contour.empty() && match) {
//        reduceDistance(from, polynode.Contour);
//        polynode.Contour.push_back(polynode.Contour.front());
//        paths.push_back(std::move(polynode.Contour));
//    }

//    for (size_t i = 0; i < polynode.ChildCount(); ++i)
//        addPolyNodeToPaths(*polynode.Childs[i], nodetype, paths);
//}

void ProfileCreator::reduceDistance(IntPoint& from, Path& to)
{
    double d = std::numeric_limits<double>::max();
    int ctr2 = 0, idx = 0;
    for (auto pt2 : to) {
        if (auto tmp = from.distToSq(pt2); d > tmp) {
            d = tmp;
            idx = ctr2;
        }
        ++ctr2;
    }
    std::rotate(to.begin(), to.begin() + idx, to.end());
    from = to.back();
}

void ProfileCreator::reduceDistance2(IntPoint& from, std::vector<PolyNode*> to)
{
}

void ProfileCreator::polyTreeToPaths(PolyTree& polytree, Paths& rpaths)
{
    rpaths.clear();
    rpaths.reserve(polytree.Total());

    std::function<void(PolyNode&, ProfileCreator::NodeType)> addPolyNodeToPaths;

    if (!Settings::profileSort()) { //Grouping by nesting

        markPolyNodeByNesting(polytree);

        std::map<int, Paths> pathsMap;
        addPolyNodeToPaths = [&addPolyNodeToPaths, &pathsMap](PolyNode& polynode, ProfileCreator::NodeType nodetype) {
            bool match = true;
            if (nodetype == ntClosed)
                match = !polynode.IsOpen();
            else if (nodetype == ntOpen)
                return;

            if (!polynode.Contour.empty() && match)
                pathsMap[polynode.Nesting].emplace_back(std::move(polynode.Contour));

            for (auto node : polynode.Childs)
                addPolyNodeToPaths(*node, nodetype);
        };
        addPolyNodeToPaths(polytree, ntClosed /*ntAny*/);

        pathsMap.extract(pathsMap.begin());

        for (auto& [nest, paths] : pathsMap) {
            qDebug() << __FUNCTION__ << "nest" << nest << paths.size();
            if (paths.size() > 1)
                sortBE(paths);
            rpaths.append(paths);
        }
    } else { //Grouping by nesting depth
        sortPolyNodeByNesting(polytree);
        IntPoint from = App::settings().mkrZeroOffset();
        std::function<void(PolyNode&, ProfileCreator::NodeType)> addPolyNodeToPaths =
            [&addPolyNodeToPaths, &rpaths, &from, this](PolyNode& polynode, ProfileCreator::NodeType nodetype) {
                bool match = true;
                if (nodetype == ntClosed)
                    match = !polynode.IsOpen();
                else if (nodetype == ntOpen)
                    return;

                if (!polynode.Contour.empty() && match && polynode.Nesting > 2) {
                    reduceDistance(from, polynode.Contour);
                    rpaths.emplace_back(std::move(polynode.Contour));
                }

                //                std::map<int, std::vector<PolyNode*>, std::greater<>> map;
                //                for (auto node : polynode.Childs)
                //                    map[node->Nesting].emplace_back(node);
                //                size_t i = polynode.ChildCount();
                //                for (auto& [nest, nodes] : map) {
                //                    for (auto node : nodes)
                //                        polynode.Childs[--i] = node;
                //                }

                for (auto node : polynode.Childs)
                    addPolyNodeToPaths(*node, nodetype);
            };

        addPolyNodeToPaths(polytree, ntClosed /*ntAny*/);
    }
}

//void ProfileCreator::closedPathsFromPolyTree(PolyTree& polytree, Paths& paths)
//{
//    paths.resize(0);
//    paths.reserve(polytree.Total());
//    addPolyNodeToPaths(polytree, ntClosed, paths);
//}

//void ProfileCreator::openPathsFromPolyTree(const PolyTree& polytree, Paths& paths)
//{
//    paths.resize(0);
//    paths.reserve(polytree.Total());
//    //Open paths are top level only, so ...
//    for (size_t i = 0; i < polytree.ChildCount(); ++i)
//        if (polytree.Childs[i]->IsOpen())
//            paths.push_back(polytree.Childs[i]->Contour);
//}
}
