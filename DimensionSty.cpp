#include"DimensionSty.h"
void  DimensionsCreateTool::InstallNewInstance()
{
	DimensionsCreateTool *tool = new DimensionsCreateTool(0, 0);

	tool->InstallTool();
}
void DimensionsCreateTool::_OnPostInstall()
{
	m_hitPath = NULL;
	m_toolState = DimensionsCreateToolState::TOOLSTATE_Locating;
	m_dimPoints.clear();
	__super::_OnPostInstall();
}
bool DimensionsCreateTool::_OnDataButton(DgnButtonEventCR ev)
{
	//View number required for creating DimCreateData object used for creating dimension element
	m_iView = ev.GetViewNum();
	//Finishes dimension if toolState is Dynamics
	if (m_toolState == DimensionsCreateToolState::TOOLSTATE_Dynamics)
	{
		FinishDimension(ev);
	}
	else 
	{
		m_hitPath = _DoLocate(ev, true, ComponentMode::Innermost);
		if (!m_hitPath)
			return false;
		m_toolState = DimensionsCreateToolState::TOOLSTATE_Dynamics;
		_BeginDynamics();
	}
	//Saves the hitPath 
	
	return false;
}
void DimensionsCreateTool::_OnRestartTool()
{
	InstallNewInstance();
}
void DimensionsCreateTool::_OnDynamicFrame(DgnButtonEventCR ev)
{
	EditElementHandle       elem;

	if (SUCCESS != CreateDimension(elem, *ev.GetPoint()))
		return;
	RedrawElems redrawElems;
	redrawElems.SetDrawMode(DRAW_MODE_TempDraw);
	redrawElems.SetDrawPurpose(DrawPurpose::Dynamics);
	redrawElems.SetViewport(ev.GetViewport());
	redrawElems.SetRedrawOp(this);
	redrawElems.DoRedraw(elem);
}
bool DimensionsCreateTool::_OnResetButton(DgnButtonEventCR ev)
{
	_OnRestartTool();
	return true;
}
bool DimensionsCreateTool::ValidateSelection(HitPathCP hitPath)
{
	//Gets the ElementHandle from the hitPath
	ElementHandle eleHandle(hitPath->GetHeadElem(), nullptr);
	//Checks whether selected element is line element or not
	return (LINE_ELM == eleHandle.GetElementType()) ? true : false;
}
bool DimensionsCreateTool::_OnPostLocate(HitPathCP path, WStringR cantAcceptReason)
{

	if (!DgnElementSetTool::_OnPostLocate(path, cantAcceptReason))
		return false;

	// Allow only line elements to be selected.
	return (ValidateSelection(path)) ? true : false;
}
/*---------------------------------------------------------------------------------**//**
* Returns the start and end points of the selected element.
*
*  @param[in]     hitPath    HitPath to get the segment points
*
* @bsimethod                                                              Bentley Systems
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DPoint3d> DimensionsCreateTool::GetSegmentPoints(HitPathCP hitPath)
{
	DSegment3d              hitSeg;
	bvector<DPoint3d>       segmentPts;
	DPoint3d                orgPoint, endPoint;

	//Gets the points for dimension element from selected line element
	hitPath->GetLinearParameters(&hitSeg, nullptr, nullptr);

	ProjectPointsTo2dIfNeeded((DPoint3d*)&hitSeg, 2);

	hitSeg.GetStartPoint(orgPoint);
	hitSeg.GetEndPoint(endPoint);

	segmentPts.push_back(orgPoint);
	segmentPts.push_back(endPoint);

	return segmentPts;
}
/*---------------------------------------------------------------------------------**//**
* Projects the points to 2D if creating dimension element in 2D model
*
* @param[out]     points     Points to be projected to 2D
* @param[in]      numPoints  Number of points to be projected to 2D
*
* @bsimethod                                                              Bentley Systems
+---------------+---------------+---------------+---------------+---------------+------*/
void DimensionsCreateTool::ProjectPointsTo2dIfNeeded(DPoint3d   *points, int numPoints)
{

	if (!ISessionMgr::GetActiveDgnModelP()->Is3d())
		for (int iPoint = 0; iPoint < numPoints; iPoint++)
			points[iPoint].z = 0.0;
}
void DimensionsCreateTool::FinishDimension(DgnButtonEventCR ev)
{
	EditElementHandle       elem;

	if (SUCCESS == CreateDimension(elem, *ev.GetPoint()))
	{
		//Apply active settings to dimension element
		//ElementPropertyUtils::ApplyActiveSettings(elem);

		//Adds dimension element to the model
		if (SUCCESS != elem.AddToModel())
			return;

		_OnReinitialize();
	}
}
StatusInt DimensionsCreateTool::CreateDimension(EditElementHandleR elem, int dimView)
{
	RotMatrix           dimRMatrix;//旋转矩阵

	dimRMatrix.InitIdentity();// Initilize matrix 初始化旋转矩阵

	//Creates object for DimCreateData , used for creating dimension element
	DimCreateData dimDataObj(dimRMatrix, dimView, m_alignment);

	//Creates dimension  element 
	return (SUCCESS != DimensionHandler::CreateDimensionElement(elem, dimDataObj, DimensionType::SizeArrow, ACTIVEMODEL->Is3d(), *ACTIVEMODEL)) ? ERROR : SUCCESS;
}
/*---------------------------------------------------------------------------------**//**
* @description   Creates the dimension element by using the DimcreateData object, dimType = SizeArrow,
*                points to insert for dimensions and DgnModel reference.
*
* @param[out]     dimElem    Dimension element to be created
* @param[in]      point      Extension point for placing dimension element
*
* @bsimethod                                                              Bentley Systems
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DimensionsCreateTool::CreateDimension(EditElementHandleR dimElem, DPoint3dCR point)
{
	bvector<DPoint3d>       dimPoints;
	DPoint3d                startPoint;
	//Sets the dimPoints , depending upon the dimension type	
	mdlHitPath_getHitPoint(m_hitPath, startPoint);
	dimPoints = GetSegmentPoints(m_hitPath);
	if (SUCCESS != CreateDimension(dimElem, m_iView))
		return ERROR;

	double              witLength;
	DVec3d              delta, corner, yvec;
	RotMatrix           rMatrix;

	//Computes the componentwise difference between two points or vectors
	//计算两个点或向量之间的分量差异
	mdlVec_subtractPoint(&delta, &point, &startPoint);
	//Computes the (bvector) sum of components in two input points or vectors.
	//计算两个输入点或向量中分量的（bvector）和。
	mdlVec_addPoint(&corner, &dimPoints.front(), &delta);

	//Creates Rotation matrix创建旋转矩阵
	mdlDim_defineRotMatrix(&rMatrix, dimElem.GetElementP(), &dimPoints.front(), &corner, &dimPoints.back());
	mdlRMatrix_getColumnVector(&yvec, &rMatrix, 1);

	//Gets witness length 
	mdlVec_subtractPoint(&delta, &point, &dimPoints.front());
	witLength = mdlVec_dotProduct(&yvec, &delta);

	//Inserts points for dimension element
	InsertPoints(dimElem, dimPoints);

	//IDimensionEdit required to set the dimension element height
	IDimensionEdit* dimEdit = dynamic_cast <IDimensionEdit*> (&dimElem.GetHandler());
	if (nullptr == dimEdit)
		return ERROR;

	dimEdit->SetHeight(dimElem, witLength);

	return SUCCESS;
}
bool DimensionsCreateTool::InsertPoints(EditElementHandleR dimElem, bvector<DPoint3d> points)
{
	//Gets the IDimensionEdit handler which is required to insert points
	IDimensionEdit* dimEdit = dynamic_cast <IDimensionEdit*> (&dimElem.GetHandler());
	if (nullptr == dimEdit)
		return false;
	//iterate through all the points in the bvector<DPOint3d> points and insert each point
	for (bvector <DPoint3d>::const_iterator iter = points.begin(); iter != points.end(); ++iter)
		dimEdit->InsertPoint(dimElem, &(*iter), nullptr, *DimensionStyle::GetActive(), 0);
	return true;
}
DimCreateData::DimCreateData(RotMatrixCR rMatrix, int vn, int alignment)
{
	m_dimRMatrix = rMatrix;
	m_viewNumber = vn;
	m_textStyle = DgnTextStyle::GetActive();
	static DgnFileP  pActiveDgnFile = mdlDgnFileObj_getMasterFile();
	m_dimStyle = DimensionStyle::GetSettings(*pActiveDgnFile);

	mdlRMatrix_fromView(&m_viewRMatrix, m_viewNumber, false);
	//字体
	m_dimStyle->SetBooleanProp(true, DIMSTYLE_PROP_Text_Font_BOOLINT);
	m_dimStyle->SetFontProp(50,DIMSTYLE_PROP_General_Font_FONT);
	m_dimStyle->SetIntegerProp(alignment, DIMSTYLE_PROP_General_Alignment_INTEGER);	
	//通用颜色
	m_dimStyle->SetBooleanProp(true, DIMSTYLE_PROP_General_OverrideColor_BOOLINT);
	m_dimStyle->SetColorProp(17, DIMSTYLE_PROP_General_Color_COLOR);
	//延长线颜色
	m_dimStyle->SetBooleanProp(true, DIMSTYLE_PROP_ExtensionLine_OverrideColor_BOOLINT);
	m_dimStyle->SetColorProp(27, DIMSTYLE_PROP_ExtensionLine_Color_COLOR);
	//线条样式
	m_dimStyle->SetBooleanProp(true, DIMSTYLE_PROP_General_OverrideLineStyle_BOOLINT);
	m_dimStyle->SetLineStyleProp(7, DIMSTYLE_PROP_General_LineStyle_LINESTYLE);
	//线条宽度
	m_dimStyle->SetBooleanProp(true, DIMSTYLE_PROP_General_OverrideWeight_BOOLINT);
	m_dimStyle->SetWeightProp(4, DIMSTYLE_PROP_General_Weight_WEIGHT);
	memset(&m_symbology, 0, sizeof m_symbology);
}
