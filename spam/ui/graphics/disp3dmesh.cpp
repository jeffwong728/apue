#include "disp3dmesh.h"
#include <numeric>
#include <algorithm>
#include <epoxy/gl.h>
#include <vtkPlanes.h>
#include <vtkAreaPicker.h>
#include <vtkProperty.h>
#include <vtkFieldData.h>
#include <vtkLookupTable.h>
#include <vtkCellData.h>
#include <vtkExtractEdges.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkUnstructuredGridGeometryFilter.h>
#include <vtkExtractSelectedFrustum.h>
#include <vtkPolyDataNormals.h>
#include <vtkLookupTable.h>
#include <vtkCellData.h>
#include <vtkTypeUInt64Array.h>

SPDispNodes GL3DMeshNode::MakeNew(const vtkSmartPointer<vtkPolyData> &pdSource, const vtkSmartPointer<vtkOpenGLRenderer> &renderer)
{
    SPDispNodes dispNodes;
    if (pdSource)
    {
        const vtkIdType numPolys = pdSource->GetNumberOfPolys();
        const vtkIdType numCells = pdSource->GetNumberOfCells();
        if (numCells == numPolys)
        {
            vtkNew<vtkPolyDataNormals> normalsFilter;
            normalsFilter->SetInputData(pdSource);
            normalsFilter->Update();
            auto dispNode = std::make_shared<GL3DMeshNode>(this_is_private{ 0 });
            dispNode->renderer_ = renderer;
            dispNode->poly_data_ = normalsFilter->GetOutput();;
            dispNode->SetDefaultDisplay();
            dispNodes.push_back(std::move(dispNode));
        }
    }

    return dispNodes;
}

SPDispNodes GL3DMeshNode::MakeNew(const vtkSmartPointer<vtkUnstructuredGrid> &ugSource, const vtkSmartPointer<vtkOpenGLRenderer> &renderer)
{
    if (ugSource)
    {
        vtkNew<vtkCellTypes> cellTypes;
        ugSource->GetCellTypes(cellTypes);
        for (vtkIdType cellId = 0; cellId < cellTypes->GetNumberOfTypes(); ++cellId)
        {
            const auto cellType = cellTypes->GetCellType(cellId);
            if (VTK_TETRA != cellType &&
                VTK_HEXAHEDRON != cellType &&
                VTK_WEDGE != cellType &&
                VTK_QUADRATIC_TETRA != cellType &&
                VTK_QUADRATIC_HEXAHEDRON != cellType &&
                VTK_QUADRATIC_WEDGE != cellType)
            {
                return SPDispNodes();
            }
        }

        vtkSmartPointer<vtkUnstructuredGridGeometryFilter> geoFilter = vtkSmartPointer<vtkUnstructuredGridGeometryFilter>::New();
        geoFilter->SetPassThroughCellIds(true);
        geoFilter->SetPassThroughPointIds(true);
        geoFilter->SetInputData(ugSource);

        vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
        surfaceFilter->SetNonlinearSubdivisionLevel(1);
        surfaceFilter->SetPassThroughCellIds(false);
        surfaceFilter->SetPassThroughPointIds(false);
        surfaceFilter->SetInputConnection(geoFilter->GetOutputPort(0));

        vtkNew<vtkPolyDataNormals> normalsFilter;
        normalsFilter->SetInputConnection(surfaceFilter->GetOutputPort());
        normalsFilter->Update();

        SPDispNodes dispNodes;
        auto dispNode = std::make_shared<GL3DMeshNode>(this_is_private{ 0 });
        dispNode->renderer_ = renderer;
        dispNode->poly_data_ = normalsFilter->GetOutput();
        dispNode->SetDefaultDisplay();

        vtkNew<vtkExtractEdges> extractEdges;
        extractEdges->SetInputConnection(geoFilter->GetOutputPort(0));
        extractEdges->Update();

        dispNode->elem_edge_poly_data_ = extractEdges->GetOutput();

        vtkNew<vtkPolyDataMapper> edgeMapper;
        edgeMapper->SetInputData(dispNode->elem_edge_poly_data_);

        dispNode->elem_edge_actor_ = vtkSmartPointer<vtkActor>::New();
        dispNode->elem_edge_actor_->SetMapper(edgeMapper);
        dispNode->elem_edge_actor_->GetProperty()->SetDiffuse(1.0);
        dispNode->elem_edge_actor_->GetProperty()->SetSpecular(0.3);
        dispNode->elem_edge_actor_->GetProperty()->SetSpecularPower(60.0);
        dispNode->elem_edge_actor_->GetProperty()->SetOpacity(1.0);
        dispNode->elem_edge_actor_->GetProperty()->SetLineWidth(1.5f);
        dispNode->elem_edge_actor_->GetProperty()->SetPointSize(5);
        dispNode->elem_edge_actor_->VisibilityOff();
        dispNode->renderer_->AddActor(dispNode->elem_edge_actor_);
        dispNodes.push_back(std::move(dispNode));

        return dispNodes;
    }

    return SPDispNodes();
}

GL3DMeshNode::GL3DMeshNode(const this_is_private&)
    : GLDispNode(GLDispNode::this_is_private{ 0 }, kENTITY_TYPE_3D_MESH)
{
}

GL3DMeshNode::~GL3DMeshNode()
{
    renderer_->RemoveActor(actor_);
    if (elem_edge_actor_)
    {
        renderer_->RemoveActor(elem_edge_actor_);
    }
}

const vtkColor4ub GL3DMeshNode::GetColor() const
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

const std::string GL3DMeshNode::GetName() const
{
    return std::string("Node");
}

void GL3DMeshNode::SetVisible(const int visible)
{
    if (actor_)
    {
        actor_->SetVisibility(visible);
    }

    if (elem_edge_actor_ && (kGREP_VTK_WIREFRAME == representation_ || kGREP_SURFACE_WITH_EDGE == representation_))
    {
        elem_edge_actor_->SetVisibility(visible);
    }
}

void GL3DMeshNode::ShowNode(const int visible)
{
    if (elem_edge_actor_ && elem_edge_actor_->GetVisibility())
    {
        elem_edge_actor_->GetProperty()->SetEdgeVisibility(visible);
        elem_edge_actor_->GetProperty()->SetVertexVisibility(visible);
    }
}

void GL3DMeshNode::SetRepresentation(const int rep)
{
    representation_ = rep;
    if (kGREP_VTK_WIREFRAME == representation_)
    {
        if (!elem_edge_poly_data_)
        {
            CreateElementEdgeActor();
        }

        if (actor_)
        {
            elem_edge_actor_->GetProperty()->SetVertexColor(actor_->GetProperty()->GetVertexColor());
            elem_edge_actor_->GetProperty()->SetDiffuseColor(actor_->GetProperty()->GetDiffuseColor());
        }

        if (elem_edge_actor_ && !elem_edge_actor_->GetVisibility())
        {
            elem_edge_actor_->VisibilityOn();
        }

        if (actor_ && actor_->GetVisibility())
        {
            actor_->VisibilityOff();
        }
    }
    else if (kGREP_SURFACE_WITH_EDGE == representation_)
    {
        if (!elem_edge_poly_data_)
        {
            CreateElementEdgeActor();
        }

        if (actor_)
        {
            elem_edge_actor_->GetProperty()->SetVertexColor(actor_->GetProperty()->GetVertexColor());
            elem_edge_actor_->GetProperty()->SetDiffuseColor(actor_->GetProperty()->GetEdgeColor());
        }

        if (elem_edge_actor_ && !elem_edge_actor_->GetVisibility())
        {
            elem_edge_actor_->VisibilityOn();
        }

        if (actor_ && !actor_->GetVisibility())
        {
            actor_->VisibilityOn();
        }
    }
    else
    {
        if (elem_edge_actor_ && elem_edge_actor_->GetVisibility())
        {
            elem_edge_actor_->VisibilityOff();
        }

        if (actor_ && !actor_->GetVisibility())
        {
            actor_->VisibilityOn();
        }
    }
}

void GL3DMeshNode::SetCellColor(const double *c)
{
    if (actor_)
    {
        actor_->GetProperty()->SetDiffuseColor(c);
        vtkLookupTable *lut = vtkLookupTable::SafeDownCast(mapper_->GetLookupTable());
        if (lut)
        {
            lut->SetTableValue(0, c);
        }
    }

    if (kGREP_VTK_WIREFRAME == representation_ && elem_edge_actor_)
    {
        elem_edge_actor_->GetProperty()->SetDiffuseColor(c);
    }
}

void GL3DMeshNode::SetEdgeColor(const double *c)
{
    if (actor_)
    {
        actor_->GetProperty()->SetEdgeColor(c);
    }

    if (elem_edge_actor_)
    {
        elem_edge_actor_->GetProperty()->SetEdgeColor(c);
    }
}

void GL3DMeshNode::SetNodeColor(const double *c)
{
    if (actor_)
    {
        actor_->GetProperty()->SetVertexColor(c);
    }

    if (elem_edge_actor_)
    {
        elem_edge_actor_->GetProperty()->SetVertexColor(c);
    }
}

vtkIdType GL3DMeshNode::Select3DCells(vtkPlanes *frustum)
{
    return SelectFacets(frustum);
}

void GL3DMeshNode::SetDefaultDisplay()
{
    const std::string arrName("vtkOriginalCellIds");
    SortFacets(poly_data_, arrName);

    mapper_ = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper_->SetInputData(poly_data_);

    actor_ = vtkSmartPointer<vtkActor>::New();
    actor_->SetMapper(mapper_);

    actor_->GetProperty()->SetDiffuse(1.0);
    actor_->GetProperty()->SetSpecular(0.3);
    actor_->GetProperty()->SetSpecularPower(60.0);
    actor_->GetProperty()->SetOpacity(1.0);
    actor_->GetProperty()->SetLineWidth(1.f);
    actor_->GetProperty()->SetPointSize(5);

    vtkSmartPointer<vtkNamedColors> colors = vtkSmartPointer<vtkNamedColors>::New();
    vtkSmartPointer<vtkIntArray> colorIndexs = vtkSmartPointer<vtkIntArray>::New();
    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetNumberOfTableValues(4);
    lut->Build();
    lut->SetTableValue(0, colors->GetColor4d("Wheat").GetData());
    lut->SetTableValue(1, colors->GetColor4d("DarkOrange").GetData());
    lut->SetTableValue(2, colors->GetColor4d("Gold").GetData());
    lut->SetTableValue(3, colors->GetColor4d("Orange").GetData());
    mapper_->SetScalarRange(0, 3);
    mapper_->SetLookupTable(lut);

    colorIndexs->SetNumberOfTuples(poly_data_->GetNumberOfCells());
    colorIndexs->FillValue(0);

    poly_data_->GetCellData()->SetScalars(colorIndexs);
    renderer_->AddActor(actor_);

    vtkNew<vtkTypeUInt64Array> guids;
    guids->SetName("GUID_TAG");
    guids->SetNumberOfComponents(2);
    guids->SetNumberOfTuples(1);
    guids->SetValue(0, guid_.part1);
    guids->SetValue(1, guid_.part2);
    poly_data_->GetFieldData()->AddArray(guids);

    cell_loc_ = vtkSmartPointer<vtkCellLocator>::New();
    cell_loc_->SetDataSet(poly_data_);
    cell_loc_->Update();
}

void GL3DMeshNode::CreateElementEdgeActor()
{
    vtkNew<vtkExtractEdges> extractEdges;
    extractEdges->SetInputData(poly_data_);
    extractEdges->Update();

    elem_edge_poly_data_ = extractEdges->GetOutput();

    vtkNew<vtkPolyDataMapper> edgeMapper;
    edgeMapper->SetInputData(elem_edge_poly_data_);

    elem_edge_actor_ = vtkSmartPointer<vtkActor>::New();
    elem_edge_actor_->SetMapper(edgeMapper);

    elem_edge_actor_->GetProperty()->SetDiffuse(1.0);
    elem_edge_actor_->GetProperty()->SetSpecular(0.3);
    elem_edge_actor_->GetProperty()->SetSpecularPower(60.0);
    elem_edge_actor_->GetProperty()->SetOpacity(1.0);
    elem_edge_actor_->GetProperty()->SetLineWidth(1.5f);
    elem_edge_actor_->GetProperty()->SetPointSize(5);

    renderer_->AddActor(elem_edge_actor_);
}
