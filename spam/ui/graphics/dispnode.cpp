#include "dispnode.h"
#include <numeric>
#include <algorithm>
#include <epoxy/gl.h>
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
            auto dispNode = std::make_shared<GLDispNode>(this_is_private{ 0 }, DisplayEntityType::kDET_VERTS);
            dispNode->renderer_ = renderer;
            dispNode->poly_data_ = pdSource;
            dispNode->SetDefaultDisplay();
            dispNodes.push_back(std::move(dispNode));
        }
        else if (numCells == numLines)
        {
            auto dispNode = std::make_shared<GLDispNode>(this_is_private{ 0 }, DisplayEntityType::kDET_LINES);
            dispNode->renderer_ = renderer;
            dispNode->poly_data_ = pdSource;
            dispNode->SetDefaultDisplay();
            dispNodes.push_back(std::move(dispNode));
        }
        else if (numCells == numPolys)
        {
            auto dispNode = std::make_shared<GLDispNode>(this_is_private{ 0 }, DisplayEntityType::kDET_POLYS);
            dispNode->renderer_ = renderer;
            dispNode->poly_data_ = pdSource;
            dispNode->SetDefaultDisplay();
            dispNodes.push_back(std::move(dispNode));
        }
        else if (numCells == numStrips)
        {
            auto dispNode = std::make_shared<GLDispNode>(this_is_private{ 0 }, DisplayEntityType::kDET_STRIPS);
            dispNode->renderer_ = renderer;
            dispNode->poly_data_ = pdSource;
            dispNode->SetDefaultDisplay();
            dispNodes.push_back(std::move(dispNode));
        }
        else
        {
            if (numVerts)
            {
                vtkNew<vtkIdList> idList;
                vtkNew<vtkCellArray> cells;
                idList->SetNumberOfIds(numVerts);
                std::iota(idList->begin(), idList->end(), 0);

                auto dispNode = std::make_shared<GLDispNode>(this_is_private{ 0 }, DisplayEntityType::kDET_VERTS);
                dispNode->renderer_ = renderer;
                dispNode->poly_data_ = vtkSmartPointer<vtkPolyData>::New();
                dispNode->poly_data_->SetVerts(cells);
                dispNode->poly_data_->GetVerts()->AllocateExact(numVerts, pdSource->GetVerts()->GetNumberOfConnectivityIds());
                dispNode->poly_data_->CopyCells(pdSource, idList);
                dispNode->SetDefaultDisplay();
                dispNodes.push_back(std::move(dispNode));
            }

            if (numLines)
            {
                vtkNew<vtkIdList> idList;
                vtkNew<vtkCellArray> cells;
                idList->SetNumberOfIds(numLines);
                std::iota(idList->begin(), idList->end(), numVerts);

                auto dispNode = std::make_shared<GLDispNode>(this_is_private{ 0 }, DisplayEntityType::kDET_LINES);
                dispNode->renderer_ = renderer;
                dispNode->poly_data_ = vtkSmartPointer<vtkPolyData>::New();
                dispNode->poly_data_->SetLines(cells);
                dispNode->poly_data_->GetLines()->AllocateExact(numLines, pdSource->GetLines()->GetNumberOfConnectivityIds());
                dispNode->poly_data_->CopyCells(pdSource, idList);
                dispNode->SetDefaultDisplay();
                dispNodes.push_back(std::move(dispNode));
            }

            if (numPolys)
            {
                vtkNew<vtkIdList> idList;
                vtkNew<vtkCellArray> cells;
                idList->SetNumberOfIds(numPolys);
                std::iota(idList->begin(), idList->end(), numVerts + numLines);

                auto dispNode = std::make_shared<GLDispNode>(this_is_private{ 0 }, DisplayEntityType::kDET_POLYS);
                dispNode->renderer_ = renderer;
                dispNode->poly_data_ = vtkSmartPointer<vtkPolyData>::New();
                dispNode->poly_data_->SetPolys(cells);
                dispNode->poly_data_->GetPolys()->AllocateExact(numPolys, pdSource->GetPolys()->GetNumberOfConnectivityIds());
                dispNode->poly_data_->CopyCells(pdSource, idList);
                dispNode->SetDefaultDisplay();
                dispNodes.push_back(std::move(dispNode));
            }

            if (numStrips)
            {
                vtkNew<vtkIdList> idList;
                vtkNew<vtkCellArray> cells;
                idList->SetNumberOfIds(numStrips);
                std::iota(idList->begin(), idList->end(), numVerts + numLines + numPolys);

                auto dispNode = std::make_shared<GLDispNode>(this_is_private{ 0 }, DisplayEntityType::kDET_STRIPS);
                dispNode->renderer_ = renderer;
                dispNode->poly_data_ = vtkSmartPointer<vtkPolyData>::New();
                dispNode->poly_data_->SetStrips(cells);
                dispNode->poly_data_->GetStrips()->AllocateExact(numStrips, pdSource->GetStrips()->GetNumberOfConnectivityIds());
                dispNode->poly_data_->CopyCells(pdSource, idList);
                dispNode->SetDefaultDisplay();
                dispNodes.push_back(std::move(dispNode));
            }
        }
    }

    return dispNodes;
}

SPDispNodes GLDispNode::MakeNew(const vtkSmartPointer<vtkUnstructuredGrid> &ugSource, const vtkSmartPointer<vtkOpenGLRenderer> &renderer)
{
    if (ugSource)
    {
        vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
        surfaceFilter->SetNonlinearSubdivisionLevel(1);
        surfaceFilter->SetInputData(ugSource);
        surfaceFilter->Update();

        return GLDispNode::MakeNew(surfaceFilter->GetOutput(), renderer);
    }

    return SPDispNodes();
}

GLDispNode::GLDispNode(const this_is_private&, const DisplayEntityType type)
    : guid_(GLGUID::MakeNew()), type_(type)
{
}

GLDispNode::~GLDispNode()
{
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
    case DisplayEntityType::kDET_VERTS: return std::string("Node");
    case DisplayEntityType::kDET_LINES: return std::string("Curve");
    case DisplayEntityType::kDET_POLYS: return std::string("Surface");
    case DisplayEntityType::kDET_STRIPS: return std::string("Surface");
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

void GLDispNode::SetCellColor(const double *c)
{
    if (actor_)
    {
        actor_->GetProperty()->SetDiffuseColor(c);
    }
}

void GLDispNode::SetEdgeColor(const double *c)
{
    if (DisplayEntityType::kDET_POLYS == type_ ||
        DisplayEntityType::kDET_STRIPS == type_)
    {
        if (actor_)
        {
            actor_->GetProperty()->SetEdgeColor(c);
        }
    }
}

void GLDispNode::SetNodeColor(const double *c)
{
    if (DisplayEntityType::kDET_LINES == type_ ||
        DisplayEntityType::kDET_POLYS == type_ ||
        DisplayEntityType::kDET_STRIPS == type_)
    {
        if (actor_)
        {
            actor_->GetProperty()->SetVertexColor(c);
        }
    }
}

void GLDispNode::SetDefaultDisplay()
{
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(poly_data_);

    actor_ = vtkSmartPointer<vtkActor>::New();
    actor_->SetMapper(mapper);

    if (DisplayEntityType::kDET_LINES == type_ ||
        DisplayEntityType::kDET_POLYS == type_ ||
        DisplayEntityType::kDET_STRIPS == type_)
    {
        actor_->GetProperty()->VertexVisibilityOn();
        actor_->GetProperty()->EdgeVisibilityOn();
    }

    actor_->GetProperty()->SetDiffuse(1.0);
    actor_->GetProperty()->SetSpecular(0.3);
    actor_->GetProperty()->SetSpecularPower(60.0);
    actor_->GetProperty()->SetOpacity(1.0);
    actor_->GetProperty()->SetLineWidth(1.5);
    actor_->GetProperty()->SetPointSize(5);

    renderer_->AddActor(actor_);
}
