#include "dispnode.h"
#include "disp0dmesh.h"
#include "disp1dmesh.h"
#include "disp2dmesh.h"
#include "disp3dmesh.h"
#include <wx/log.h>
#include <numeric>
#include <algorithm>
#include <epoxy/gl.h>
#include <vtkCellType.h>
#include <vtkGenericCell.h>
#include <vtkProperty.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkUnstructuredGridGeometryFilter.h>

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

            if (VTK_TRIANGLE == cellType ||
                VTK_QUAD == cellType ||
                VTK_QUADRATIC_TRIANGLE == cellType ||
                VTK_QUADRATIC_QUAD == cellType)
            {
                num2Ds += 1;
            }
        }

        typesInfo += wxT("]");
        wxLogMessage(typesInfo);

        if (num2Ds == cellTypes->GetNumberOfTypes())
        {
            return GL2DMeshNode::MakeNew(ugSource, renderer);
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
    case kENTITY_TYPE_1D_MESH: return std::string("Curve");
    case kENTITY_TYPE_2D_MESH: return std::string("Surface");
    case kENTITY_TYPE_3D_MESH: return std::string("Surface");
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
