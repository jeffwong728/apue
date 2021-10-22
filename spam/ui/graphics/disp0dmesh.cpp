#include "disp0dmesh.h"
#include <numeric>
#include <algorithm>
#include <epoxy/gl.h>
#include <vtkProperty.h>
#include <vtkFieldData.h>
#include <vtkLookupTable.h>
#include <vtkCellData.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkUnstructuredGridGeometryFilter.h>
#include <vtkTypeUInt64Array.h>

SPDispNodes GL0DMeshNode::MakeNew(const vtkSmartPointer<vtkPolyData> &pdSource, const vtkSmartPointer<vtkOpenGLRenderer> &renderer)
{
    SPDispNodes dispNodes;
    if (pdSource)
    {
        const vtkIdType numVerts = pdSource->GetNumberOfVerts();
        const vtkIdType numCells = pdSource->GetNumberOfCells();
        if (numCells == numVerts)
        {
            auto dispNode = std::make_shared<GL0DMeshNode>(this_is_private{ 0 });
            dispNode->renderer_ = renderer;
            dispNode->poly_data_ = pdSource;
            dispNode->SetDefaultDisplay();
            dispNodes.push_back(std::move(dispNode));
        }
    }

    return dispNodes;
}

SPDispNodes GL0DMeshNode::MakeNew(const vtkSmartPointer<vtkUnstructuredGrid> &ugSource, const vtkSmartPointer<vtkOpenGLRenderer> &renderer)
{
    if (ugSource)
    {
        vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
        surfaceFilter->SetNonlinearSubdivisionLevel(1);
        surfaceFilter->SetInputData(ugSource);
        surfaceFilter->Update();

        return GL0DMeshNode::MakeNew(surfaceFilter->GetOutput(), renderer);
    }

    return SPDispNodes();
}

GL0DMeshNode::GL0DMeshNode(const this_is_private&)
    : GLDispNode(GLDispNode::this_is_private{ 0 }, kENTITY_TYPE_0D_MESH)
{
}

GL0DMeshNode::~GL0DMeshNode()
{
    renderer_->RemoveActor(actor_);
}

const vtkColor4ub GL0DMeshNode::GetColor() const
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

const std::string GL0DMeshNode::GetName() const
{
    return std::string("Node");
}

void GL0DMeshNode::SetVisible(const int visible)
{
    if (actor_)
    {
        actor_->SetVisibility(visible);
    }
}

void GL0DMeshNode::ShowNode(const int visible)
{
    if (actor_)
    {
        actor_->GetProperty()->SetVertexVisibility(visible);
        actor_->GetProperty()->SetEdgeVisibility(visible);
    }
}

void GL0DMeshNode::SetRepresentation(const int vtkNotUsed(rep))
{
}

void GL0DMeshNode::SetCellColor(const double *c)
{
    if (actor_)
    {
        actor_->GetProperty()->SetDiffuseColor(c);
    }
}

void GL0DMeshNode::SetEdgeColor(const double *vtkNotUsed(c))
{
}

void GL0DMeshNode::SetNodeColor(const double *c)
{
    if (actor_)
    {
        actor_->GetProperty()->SetVertexColor(c);
    }
}

void GL0DMeshNode::SetDefaultDisplay()
{
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
}
