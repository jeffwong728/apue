#include "dispnode.h"
#include "disp0dmesh.h"
#include "disp1dmesh.h"
#include "disp2dmesh.h"
#include "disp3dmesh.h"
#include <wx/log.h>
#include <ui/spam.h>
#include <numeric>
#include <algorithm>
#include <epoxy/gl.h>
#include <vtkCellType.h>
#include <vtkGenericCell.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkSignedCharArray.h>
#include <vtkIdTypeArray.h>
#include <vtkProperty.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkUnstructuredGridGeometryFilter.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkExtractSelectedFrustum.h>
#include <vtksys/SystemTools.hxx>
#include <vtkTypeUInt64Array.h>

SPDispNodes GLDispNode::MakeNew(const vtkSmartPointer<vtkPolyData> &pdSource, const vtkSmartPointer<vtkOpenGLRenderer> &renderer)
{
    SPDispNodes dispNodes;
    if (pdSource)
    {
        const vtkIdType numVerts = pdSource->GetNumberOfVerts();
        const vtkIdType numLines = pdSource->GetNumberOfLines();
        const vtkIdType numPolys = pdSource->GetNumberOfPolys();
        const vtkIdType numStrips = pdSource->GetNumberOfStrips();
        const vtkIdType numCells = pdSource->GetNumberOfCells();
        if (numCells == numVerts)
        {
            SPDispNodes spDispNodes = GL0DMeshNode::MakeNew(pdSource, renderer);
            dispNodes.insert(dispNodes.end(), spDispNodes.cbegin(), spDispNodes.cend());
        }
        else if (numCells == numLines)
        {
            SPDispNodes spDispNodes = GL1DMeshNode::MakeNew(pdSource, renderer);
            dispNodes.insert(dispNodes.end(), spDispNodes.cbegin(), spDispNodes.cend());
        }
        else if (numCells == numPolys)
        {
            SPDispNodes spDispNodes = GL2DMeshNode::MakeNew(pdSource, renderer);
            dispNodes.insert(dispNodes.end(), spDispNodes.cbegin(), spDispNodes.cend());
        }
        else if (numCells == numStrips)
        {
            SPDispNodes spDispNodes = GL2DMeshNode::MakeNew(pdSource, renderer);
            dispNodes.insert(dispNodes.end(), spDispNodes.cbegin(), spDispNodes.cend());
        }
        else
        {
            if (numVerts)
            {
                vtkNew<vtkIdList> idList;
                vtkNew<vtkCellArray> cells;
                idList->SetNumberOfIds(numVerts);
                std::iota(idList->begin(), idList->end(), 0);

                auto polyData = vtkSmartPointer<vtkPolyData>::New();
                polyData->SetVerts(cells);
                polyData->GetVerts()->AllocateExact(numVerts, pdSource->GetVerts()->GetNumberOfConnectivityIds());
                polyData->CopyCells(pdSource, idList);

                SPDispNodes spDispNodes = GL0DMeshNode::MakeNew(polyData, renderer);
                dispNodes.insert(dispNodes.end(), spDispNodes.cbegin(), spDispNodes.cend());
            }

            if (numLines)
            {
                vtkNew<vtkIdList> idList;
                vtkNew<vtkCellArray> cells;
                idList->SetNumberOfIds(numLines);
                std::iota(idList->begin(), idList->end(), numVerts);

                auto polyData = vtkSmartPointer<vtkPolyData>::New();
                polyData->SetLines(cells);
                polyData->GetLines()->AllocateExact(numLines, pdSource->GetLines()->GetNumberOfConnectivityIds());
                polyData->CopyCells(pdSource, idList);

                SPDispNodes spDispNodes = GL1DMeshNode::MakeNew(polyData, renderer);
                dispNodes.insert(dispNodes.end(), spDispNodes.cbegin(), spDispNodes.cend());
            }

            if (numPolys)
            {
                vtkNew<vtkIdList> idList;
                vtkNew<vtkCellArray> cells;
                idList->SetNumberOfIds(numPolys);
                std::iota(idList->begin(), idList->end(), numVerts + numLines);

                auto polyData = vtkSmartPointer<vtkPolyData>::New();
                polyData->SetPolys(cells);
                polyData->GetPolys()->AllocateExact(numPolys, pdSource->GetPolys()->GetNumberOfConnectivityIds());
                polyData->CopyCells(pdSource, idList);

                SPDispNodes spDispNodes = GL2DMeshNode::MakeNew(polyData, renderer);
                dispNodes.insert(dispNodes.end(), spDispNodes.cbegin(), spDispNodes.cend());
            }

            if (numStrips)
            {
                vtkNew<vtkIdList> idList;
                vtkNew<vtkCellArray> cells;
                idList->SetNumberOfIds(numStrips);
                std::iota(idList->begin(), idList->end(), numVerts + numLines + numPolys);

                auto polyData = vtkSmartPointer<vtkPolyData>::New();
                polyData->SetStrips(cells);
                polyData->GetStrips()->AllocateExact(numStrips, pdSource->GetStrips()->GetNumberOfConnectivityIds());
                polyData->CopyCells(pdSource, idList);

                SPDispNodes spDispNodes = GL2DMeshNode::MakeNew(polyData, renderer);
                dispNodes.insert(dispNodes.end(), spDispNodes.cbegin(), spDispNodes.cend());
            }
        }
    }

    return dispNodes;
}

SPDispNodes GLDispNode::MakeNew(const vtkSmartPointer<vtkUnstructuredGrid> &ugSource, const vtkSmartPointer<vtkOpenGLRenderer> &renderer)
{
    if (ugSource)
    {
        vtkNew<vtkCellTypes> cellTypes;
        ugSource->GetCellTypes(cellTypes);

        int num0Ds = 0;
        int num1Ds = 0;
        int num2Ds = 0;
        int num3Ds = 0;
        wxString typesInfo(wxT("UG Cell Types: ["));
        for (vtkIdType cellId =0; cellId < cellTypes->GetNumberOfTypes(); ++cellId)
        {
            const auto cellType = cellTypes->GetCellType(cellId);
            std::string cellTypeVal = std::to_string(cellType);
            std::string cellTypeName(vtkCellTypes::GetClassNameFromTypeId(cellType));

            if (cellId)
            {
                typesInfo += wxT(", (");
            }
            else
            {
                typesInfo += wxT("(");
            }

            typesInfo += cellTypeVal;
            typesInfo += wxT(", ");
            typesInfo += cellTypeName;
            typesInfo += wxT(")");

            if (VTK_VERTEX == cellType)
            {
                num0Ds += 1;
            }

            if (VTK_LINE == cellType)
            {
                num1Ds += 1;
            }

            if (VTK_TRIANGLE == cellType ||
                VTK_QUAD == cellType ||
                VTK_QUADRATIC_TRIANGLE == cellType ||
                VTK_QUADRATIC_QUAD == cellType)
            {
                num2Ds += 1;
            }

            if (VTK_TETRA == cellType ||
                VTK_HEXAHEDRON == cellType ||
                VTK_WEDGE == cellType ||
                VTK_QUADRATIC_TETRA == cellType ||
                VTK_QUADRATIC_HEXAHEDRON == cellType ||
                VTK_QUADRATIC_WEDGE == cellType)
            {
                num3Ds += 1;
            }
        }

        typesInfo += wxT("]");
        wxLogMessage(typesInfo);

        if (num0Ds == cellTypes->GetNumberOfTypes())
        {
            return GL0DMeshNode::MakeNew(ugSource, renderer);
        }
        else if (num1Ds == cellTypes->GetNumberOfTypes())
        {
            return GL1DMeshNode::MakeNew(ugSource, renderer);
        }
        else if (num2Ds == cellTypes->GetNumberOfTypes())
        {
            return GL2DMeshNode::MakeNew(ugSource, renderer);
        }
        else if (num3Ds == cellTypes->GetNumberOfTypes())
        {
            return GL3DMeshNode::MakeNew(ugSource, renderer);
        }
        else
        {
            vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
            surfaceFilter->SetNonlinearSubdivisionLevel(0);
            surfaceFilter->SetInputData(ugSource);
            surfaceFilter->Update();
            return GLDispNode::MakeNew(surfaceFilter->GetOutput(), renderer);
        }
    }

    return SPDispNodes();
}

GLDispNode::GLDispNode(const this_is_private&, const GraphicsEntityType type)
    : guid_(GLGUID::MakeNew()), type_(type)
{
}

GLDispNode::~GLDispNode()
{
    renderer_->RemoveActor(actor_);
}

const vtkColor4ub GLDispNode::GetColor() const
{
    double colors[3] = { 0 };
    if (actor_)
    {
        actor_->GetProperty()->GetDiffuseColor(colors);
    }

    unsigned char r = static_cast<unsigned char>(colors[0] * 255.0);
    unsigned char g = static_cast<unsigned char>(colors[1] * 255.0);
    unsigned char b = static_cast<unsigned char>(colors[2] * 255.0);

    return vtkColor4ub(r, g, b);
}

const std::string GLDispNode::GetName() const
{
    switch (type_)
    {
    case kENTITY_TYPE_0D_MESH: return std::string("Node");
    case kENTITY_TYPE_1D_MESH: return std::string("Curve Mesh");
    case kENTITY_TYPE_2D_MESH: return std::string("Surface Mesh");
    case kENTITY_TYPE_3D_MESH: return std::string("Solid Mesh");
    default: return std::string("Unknown");
    }
}

void GLDispNode::SetVisible(const int visible)
{
    if (actor_)
    {
        actor_->SetVisibility(visible);
    }
}

const vtkIdType GLDispNode::GetCellId(const vtkIdType facetId)
{
    vtkIdTypeArray *cellIds = vtkIdTypeArray::SafeDownCast(poly_data_->GetCellData()->GetArray("vtkOriginalCellIds"));
    if (cellIds && facetId >=0 && facetId < cellIds->GetNumberOfTuples())
    {
        return cellIds->GetValue(facetId);
    }
    return -1;
}

const vtkIdType GLDispNode::GetHighlightCellId()
{
    vtkIntArray *colorIndexs = vtkIntArray::SafeDownCast(poly_data_->GetCellData()->GetScalars());
    vtkIdTypeArray *cellIds = vtkIdTypeArray::SafeDownCast(poly_data_->GetCellData()->GetArray("vtkOriginalCellIds"));
    return -1;
}

const vtkIdType GLDispNode::HighlightCell(const vtkIdType facetId)
{
    vtkIntArray *colorIndexs = vtkIntArray::SafeDownCast(poly_data_->GetCellData()->GetScalars());
    vtkIdTypeArray *cellIds = vtkIdTypeArray::SafeDownCast(poly_data_->GetCellData()->GetArray("vtkOriginalCellIds"));

    vtkIdType numSelStatusChanged = 0;
    auto getCellId = [=](const vtkIdType facetId) { return cellIds ? cellIds->GetValue(facetId) : facetId; };
    if (facetId >= 0 && facetId < colorIndexs->GetNumberOfTuples())
    {
        const vtkIdType highlightCellId = getCellId(facetId);
        const vtkIdType numColorIndexs = colorIndexs->GetNumberOfTuples();
        for (vtkIdType cc = 0; cc < numColorIndexs; ++cc)
        {
            if (highlightCellId != getCellId(cc))
            {
                switch (colorIndexs->GetValue(cc))
                {
                case kCell_Color_Index_Highlight: colorIndexs->SetValue(cc, kCell_Color_Index_Normal); numSelStatusChanged += 1; break;
                case kCell_Color_Index_Selected_And_Highlight: colorIndexs->SetValue(cc, kCell_Color_Index_Selected); numSelStatusChanged += 1; break;
                default: break;
                }
            }
        }

        switch (colorIndexs->GetValue(facetId))
        {
        case kCell_Color_Index_Normal: colorIndexs->SetValue(facetId, kCell_Color_Index_Highlight); numSelStatusChanged += 1; break;
        case kCell_Color_Index_Selected: colorIndexs->SetValue(facetId, kCell_Color_Index_Selected_And_Highlight); numSelStatusChanged += 1; break;
        default: break;
        }

        for (vtkIdType kk = facetId - 1; kk >= 0 && highlightCellId == getCellId(kk); --kk)
        {
            switch (colorIndexs->GetValue(kk))
            {
            case kCell_Color_Index_Normal: colorIndexs->SetValue(kk, kCell_Color_Index_Highlight); numSelStatusChanged += 1; break;
            case kCell_Color_Index_Selected: colorIndexs->SetValue(kk, kCell_Color_Index_Selected_And_Highlight); numSelStatusChanged += 1; break;
            default: break;
            }
        }

        for (vtkIdType kk = facetId + 1; kk < numColorIndexs && highlightCellId == getCellId(kk); ++kk)
        {
            switch (colorIndexs->GetValue(kk))
            {
            case kCell_Color_Index_Normal: colorIndexs->SetValue(kk, kCell_Color_Index_Highlight); numSelStatusChanged += 1; break;
            case kCell_Color_Index_Selected: colorIndexs->SetValue(kk, kCell_Color_Index_Selected_And_Highlight); numSelStatusChanged += 1; break;
            default: break;
            }
        }

        if (numSelStatusChanged > 0)
        {
            colorIndexs->Modified();
        }
    }
    else if (facetId == -1)
    {
        const vtkIdType numColorIndexs = colorIndexs->GetNumberOfTuples();
        for (vtkIdType cc = 0; cc < numColorIndexs; ++cc)
        {
            switch (colorIndexs->GetValue(cc))
            {
            case kCell_Color_Index_Highlight: colorIndexs->SetValue(cc, kCell_Color_Index_Normal); numSelStatusChanged += 1; break;
            case kCell_Color_Index_Selected_And_Highlight: colorIndexs->SetValue(cc, kCell_Color_Index_Selected); numSelStatusChanged += 1; break;
            default: break;
            }
        }

        if (numSelStatusChanged > 0)
        {
            colorIndexs->Modified();
        }
    }
    else
    {
        const vtkIdType numColorIndexs = colorIndexs->GetNumberOfTuples();
        for (vtkIdType cc = 0; cc < numColorIndexs; ++cc)
        {
            switch (colorIndexs->GetValue(cc))
            {
            case kCell_Color_Index_Normal: colorIndexs->SetValue(cc, kCell_Color_Index_Highlight); numSelStatusChanged += 1; break;
            case kCell_Color_Index_Selected: colorIndexs->SetValue(cc, kCell_Color_Index_Selected_And_Highlight); numSelStatusChanged += 1; break;
            default: break;
            }
        }

        if (numSelStatusChanged > 0)
        {
            colorIndexs->Modified();
        }
    }

    return numSelStatusChanged;
}

const vtkIdType GLDispNode::SelectCell(const vtkIdType facetId)
{
    vtkIntArray *colorIndexs = vtkIntArray::SafeDownCast(poly_data_->GetCellData()->GetScalars());
    vtkIdTypeArray *cellIds = vtkIdTypeArray::SafeDownCast(poly_data_->GetCellData()->GetArray("vtkOriginalCellIds"));

    vtkIdType numDispStatusChanged = 0;
    auto getCellId = [=](const vtkIdType facetId) { return cellIds ? cellIds->GetValue(facetId) : facetId; };
    if (facetId >= 0 && facetId < colorIndexs->GetNumberOfTuples())
    {
        const vtkIdType selCellId = getCellId(facetId);
        const vtkIdType numColorIndexs = colorIndexs->GetNumberOfTuples();
        for (vtkIdType cc = 0; cc < numColorIndexs; ++cc)
        {
            if (selCellId != getCellId(cc))
            {
                switch (colorIndexs->GetValue(cc))
                {
                case kCell_Color_Index_Selected: colorIndexs->SetValue(cc, kCell_Color_Index_Normal); numDispStatusChanged += 1; break;
                case kCell_Color_Index_Selected_And_Highlight: colorIndexs->SetValue(cc, kCell_Color_Index_Highlight); numDispStatusChanged += 1; break;
                default: break;
                }
            }
        }

        switch (colorIndexs->GetValue(facetId))
        {
        case kCell_Color_Index_Normal: colorIndexs->SetValue(facetId, kCell_Color_Index_Selected); numDispStatusChanged += 1; break;
        case kCell_Color_Index_Highlight: colorIndexs->SetValue(facetId, kCell_Color_Index_Selected_And_Highlight); numDispStatusChanged += 1; break;
        default: break;
        }

        for (vtkIdType kk = facetId - 1; kk >= 0 && selCellId == getCellId(kk); --kk)
        {
            switch (colorIndexs->GetValue(kk))
            {
            case kCell_Color_Index_Normal: colorIndexs->SetValue(kk, kCell_Color_Index_Selected); numDispStatusChanged += 1; break;
            case kCell_Color_Index_Highlight: colorIndexs->SetValue(kk, kCell_Color_Index_Selected_And_Highlight); numDispStatusChanged += 1; break;
            default: break;
            }
        }

        for (vtkIdType kk = facetId + 1; kk < numColorIndexs && selCellId == getCellId(kk); ++kk)
        {
            switch (colorIndexs->GetValue(kk))
            {
            case kCell_Color_Index_Normal: colorIndexs->SetValue(kk, kCell_Color_Index_Selected); numDispStatusChanged += 1; break;
            case kCell_Color_Index_Highlight: colorIndexs->SetValue(kk, kCell_Color_Index_Selected_And_Highlight); numDispStatusChanged += 1; break;
            default: break;
            }
        }

        if (numDispStatusChanged > 0)
        {
            colorIndexs->Modified();
        }
    }
    else if (facetId == -1)
    {
        const vtkIdType numColorIndexs = colorIndexs->GetNumberOfTuples();
        for (vtkIdType cc = 0; cc < numColorIndexs; ++cc)
        {
            switch (colorIndexs->GetValue(cc))
            {
            case kCell_Color_Index_Selected: colorIndexs->SetValue(cc, kCell_Color_Index_Normal); numDispStatusChanged += 1; break;
            case kCell_Color_Index_Selected_And_Highlight: colorIndexs->SetValue(cc, kCell_Color_Index_Highlight); numDispStatusChanged += 1; break;
            default: break;
            }
        }

        if (numDispStatusChanged > 0)
        {
            colorIndexs->Modified();
        }
    }
    else
    {
        const vtkIdType numColorIndexs = colorIndexs->GetNumberOfTuples();
        for (vtkIdType cc = 0; cc < numColorIndexs; ++cc)
        {
            switch (colorIndexs->GetValue(cc))
            {
            case kCell_Color_Index_Normal: colorIndexs->SetValue(cc, kCell_Color_Index_Selected); numDispStatusChanged += 1; break;
            case kCell_Color_Index_Highlight: colorIndexs->SetValue(cc, kCell_Color_Index_Selected_And_Highlight); numDispStatusChanged += 1; break;
            default: break;
            }
        }

        if (numDispStatusChanged > 0)
        {
            colorIndexs->Modified();
        }
    }

    return numDispStatusChanged;
}

void GLDispNode::ShowNode(const int visible)
{
    if (actor_)
    {
        actor_->GetProperty()->SetVertexVisibility(visible);
        actor_->GetProperty()->SetEdgeVisibility(visible);
    }
}

void GLDispNode::SetRepresentation(const int rep)
{
    if (actor_)
    {
        if (VTK_WIREFRAME == rep)
        {
            actor_->GetProperty()->SetRepresentationToWireframe();
        }
        else
        {
            actor_->GetProperty()->SetRepresentationToSurface();
        }
    }
}

void GLDispNode::SetCellColor(const double *c)
{
    if (actor_)
    {
        actor_->GetProperty()->SetDiffuseColor(c);
    }
}

void GLDispNode::SetEdgeColor(const double *c)
{
    if (kENTITY_TYPE_2D_MESH == type_ || kENTITY_TYPE_3D_MESH == type_)
    {
        if (actor_)
        {
            actor_->GetProperty()->SetEdgeColor(c);
        }
    }
}

void GLDispNode::SetNodeColor(const double *c)
{
    if (kENTITY_TYPE_1D_MESH == type_ ||
        kENTITY_TYPE_2D_MESH == type_ ||
        kENTITY_TYPE_3D_MESH == type_)
    {
        if (actor_)
        {
            actor_->GetProperty()->SetVertexColor(c);
        }
    }
}

vtkIdType GLDispNode::Select1DCells(vtkPlanes *frustum)
{
    return 0;
}

vtkIdType GLDispNode::Select2DCells(vtkPlanes *frustum)
{
    return 0;
}

vtkIdType GLDispNode::Select3DCells(vtkPlanes *frustum)
{
    return 0;
}

bool GLDispNode::ExportVTK(const std::string &dir)
{
    if (poly_data_)
    {
        std::string filePath = dir + boost::filesystem::path::preferred_separator + GetName();
        filePath.append(".vtp");

        vtkNew<vtkXMLPolyDataWriter> writer;
        writer->SetFileName(filePath.c_str());
        writer->SetInputData(poly_data_);
        writer->SetDataModeToAscii();
        writer->Write();
    }

    return true;
}

vtkIdType GLDispNode::SelectFacets(vtkPlanes *frustum)
{
    vtkNew<vtkExtractSelectedFrustum> extractor;
    extractor->SetInputData(poly_data_);
    extractor->PreserveTopologyOn();
    extractor->SetFrustum(frustum);
    extractor->Update();

    vtkIdType numSelected = 0;
    vtkIdType numSelStatusChanged = 0;
    vtkPolyData *selPolyData = vtkPolyData::SafeDownCast(extractor->GetOutput());
    vtkIntArray *colorIndexs = vtkIntArray::SafeDownCast(poly_data_->GetCellData()->GetScalars());
    if (selPolyData)
    {
        vtkSignedCharArray *cellInsidedness = vtkSignedCharArray::SafeDownCast(selPolyData->GetCellData()->GetArray("vtkInsidedness"));
        if (cellInsidedness)
        {
            const vtkIdType numInsidedness = cellInsidedness->GetNumberOfTuples();
            if (colorIndexs && colorIndexs->GetNumberOfTuples() && numInsidedness == colorIndexs->GetNumberOfTuples())
            {
                vtkIdTypeArray *cellIds = vtkIdTypeArray::SafeDownCast(selPolyData->GetCellData()->GetArray("vtkOriginalCellIds"));
                for (vtkIdType cc = 0; cc < numInsidedness; ++cc)
                {
                    if (cellInsidedness->GetValue(cc) < 0)
                    {
                        switch (colorIndexs->GetValue(cc))
                        {
                        case kCell_Color_Index_Selected:
                            colorIndexs->SetValue(cc, kCell_Color_Index_Normal);
                            numSelStatusChanged += 1;
                            break;

                        case kCell_Color_Index_Selected_And_Highlight:
                            colorIndexs->SetValue(cc, kCell_Color_Index_Highlight);
                            numSelStatusChanged += 1;
                            break;

                        default:
                            break;
                        }
                    }
                }
                for (vtkIdType cc = 0; cc < numInsidedness; ++cc)
                {
                    if (cellInsidedness->GetValue(cc) > 0)
                    {
                        switch (colorIndexs->GetValue(cc))
                        {
                        case kCell_Color_Index_Normal:
                            colorIndexs->SetValue(cc, kCell_Color_Index_Selected);
                            numSelStatusChanged += 1;
                            break;

                        case kCell_Color_Index_Highlight:
                            colorIndexs->SetValue(cc, kCell_Color_Index_Selected_And_Highlight);
                            numSelStatusChanged += 1;
                            break;

                        default:
                            break;
                        }

                        if (cellIds)
                        {
                            const vtkIdType cellId = cellIds->GetValue(cc);
                            for (vtkIdType kk = cc - 1; kk >= 0 && cellId == cellIds->GetValue(kk); --kk)
                            {
                                switch (colorIndexs->GetValue(kk))
                                {
                                case kCell_Color_Index_Normal:
                                    colorIndexs->SetValue(kk, kCell_Color_Index_Selected);
                                    numSelStatusChanged += 1;
                                    break;

                                case kCell_Color_Index_Highlight:
                                    colorIndexs->SetValue(kk, kCell_Color_Index_Selected_And_Highlight);
                                    numSelStatusChanged += 1;
                                    break;

                                default:
                                    break;
                                }
                            }

                            for (vtkIdType kk = cc + 1; kk < numInsidedness && cellId == cellIds->GetValue(kk); ++kk)
                            {
                                switch (colorIndexs->GetValue(kk))
                                {
                                case kCell_Color_Index_Normal:
                                    colorIndexs->SetValue(kk, kCell_Color_Index_Selected);
                                    numSelStatusChanged += 1;
                                    break;

                                case kCell_Color_Index_Highlight:
                                    colorIndexs->SetValue(kk, kCell_Color_Index_Selected_And_Highlight);
                                    numSelStatusChanged += 1;
                                    break;

                                default:
                                    break;
                                }
                            }
                        }

                        numSelected += 1;
                    }
                }

                if (numSelStatusChanged > 0)
                {
                    colorIndexs->Modified();
                }

                const wxString numSelStr(std::to_string(numSelected));
                Spam::SetStatus(StatusIconType::kSIT_NONE, wxString(wxT("Selected ")) + numSelStr + wxString(wxT(" 2D cells")));

                return numSelStatusChanged;
            }
        }
    }

    if (colorIndexs)
    {
        const vtkIdType numColorIndexs = colorIndexs->GetNumberOfTuples();
        for (vtkIdType cc = 0; cc < numColorIndexs; ++cc)
        {
            switch (colorIndexs->GetValue(cc))
            {
            case kCell_Color_Index_Selected:
                colorIndexs->SetValue(cc, kCell_Color_Index_Normal);
                numSelStatusChanged += 1;
                break;

            case kCell_Color_Index_Selected_And_Highlight:
                colorIndexs->SetValue(cc, kCell_Color_Index_Highlight);
                numSelStatusChanged += 1;
                break;

            default:
                break;
            }
        }
        if (numSelStatusChanged > 0)
        {
            colorIndexs->Modified();
        }
    }

    return numSelStatusChanged;
}

void GLDispNode::SortFacets(vtkSmartPointer<vtkPolyData> &spPolyData, const std::string &arrName)
{
    if (spPolyData)
    {
        const vtkIdType numFacets = spPolyData->GetNumberOfCells();
        vtkIdTypeArray *cellIds = vtkIdTypeArray::SafeDownCast(spPolyData->GetCellData()->GetArray(arrName.c_str()));
        if (numFacets && cellIds && cellIds->Begin() && !std::is_sorted(cellIds->Begin(), cellIds->End()))
        {
            std::vector<int> indexs(numFacets);
            std::iota(indexs.begin(), indexs.end(), 0);
            vtkIdType *begIndxs = cellIds->Begin();
            std::stable_sort(indexs.begin(), indexs.end(), [=](const int i, const int j) { return begIndxs[i] < begIndxs[j]; });
            vtkCellArray *connects = spPolyData->GetPolys();
            if (!connects || !connects->GetNumberOfCells()) 
            { 
                connects = spPolyData->GetLines();
            }

            if (connects)
            {
                if (connects->IsStorage64Bit())
                {
                    vtkTypeInt64Array *offsetArray = vtkTypeInt64Array::SafeDownCast(connects->GetOffsetsArray());
                    vtkTypeInt64Array *connectivityArray = vtkTypeInt64Array::SafeDownCast(connects->GetConnectivityArray());
                    vtkIdType *pCellId = cellIds->Begin();
                    vtkTypeInt64 *pFacetOffset = offsetArray->Begin();
                    vtkTypeInt64 *pFacetConnectivity = connectivityArray->Begin();

                    std::vector<vtkTypeUInt64> buffer(connectivityArray->GetNumberOfValues());
                    vtkIdType *pBufferCellId = reinterpret_cast<vtkIdType *>(buffer.data());
                    for (vtkIdType t=0; t < numFacets; ++t)
                    {
                        const int idx = indexs[t];
                        pBufferCellId[t] = pCellId[idx];
                    }
                    std::memcpy(pCellId, pBufferCellId, sizeof(vtkIdType)*cellIds->GetNumberOfValues());

                    vtkTypeInt64 newOffset = 0;
                    std::vector<vtkTypeUInt64> obuffer(offsetArray->GetNumberOfValues());
                    vtkTypeInt64 *pBufferOffset = reinterpret_cast<vtkTypeInt64 *>(obuffer.data());
                    vtkTypeInt64 *pBufferConnectivity = reinterpret_cast<vtkTypeInt64 *>(buffer.data());
                    for (vtkIdType t = 0; t < numFacets; ++t)
                    {
                        const int idx = indexs[t];
                        const vtkTypeInt64 offset = pFacetOffset[idx];
                        const vtkTypeInt64 noffset = pFacetOffset[idx+1];
                        const vtkTypeInt64 numConnects = noffset - offset;
                        pBufferOffset[t] = newOffset;
                        for (vtkTypeInt64 f = offset, nf = newOffset; f < noffset; ++f, ++nf)
                        {
                            pBufferConnectivity[nf] = pFacetConnectivity[f];
                        }
                        newOffset += numConnects;
                    }
                    pBufferOffset[numFacets] = newOffset;
                    std::memcpy(pFacetOffset, pBufferOffset, sizeof(vtkTypeInt64)*offsetArray->GetNumberOfValues());
                    std::memcpy(pFacetConnectivity, pBufferConnectivity, sizeof(vtkTypeInt64)*connectivityArray->GetNumberOfValues());
                }
                else
                {
                    vtkTypeInt32Array *offsetArray = vtkTypeInt32Array::SafeDownCast(connects->GetOffsetsArray());
                    vtkTypeInt32Array *connectivityArray = vtkTypeInt32Array::SafeDownCast(connects->GetConnectivityArray());
                    vtkIdType *pCellId = cellIds->Begin();
                    vtkTypeInt32 *pFacetOffset = offsetArray->Begin();
                    vtkTypeInt32 *pFacetConnectivity = connectivityArray->Begin();

                    for (vtkIdType t = 0; t < numFacets; ++t)
                    {

                    }
                }
            }
        }
    }
}
