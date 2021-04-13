#pragma once
#include    <Mstn/MdlApi/MdlApi.h>
#include    <Mstn/ISessionMgr.h>
#include    <Mstn/ElementPropertyUtils.h>
#include    <DgnView/DgnElementSetTool.h>
#include    <DgnPlatform/DimensionHandler.h>
#include    <DgnPlatform/ISettings.h>
#include    "DimensionStycmd.h"
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_MSTNPLATFORM
USING_NAMESPACE_BENTLEY_MSTNPLATFORM_ELEMENT
struct       DimensionsCreateTool : DgnElementSetTool
{
private:

	enum class  DimensionsCreateToolState
	{
		TOOLSTATE_Locating = 0,
		TOOLSTATE_Dynamics = 1
	};

	int                             m_iView;
	HitPathCP                       m_hitPath;
	DimensionsCreateToolState       m_toolState;
	bvector<DPoint3d>               m_dimPoints;
	int                             m_alignment;

	DimensionsCreateTool(int toolName, int alignment) : DgnElementSetTool(toolName), m_alignment(alignment) { }

	virtual     void                _OnPostInstall() override;
	virtual     bool                _OnDataButton(DgnButtonEventCR ev) override;
	virtual     void                _OnRestartTool() override;
	virtual     void                _OnDynamicFrame(DgnButtonEventCR ev) override;
	virtual     bool                _OnResetButton(DgnButtonEventCR ev) override;
	virtual     bool                _OnPostLocate(HitPathCP path, WStringR cantAcceptReason) override;
	StatusInt                       _OnElementModify(EditElementHandleR eeh) override { return SUCCESS; }  // pure virtual method, sub-class must override!

	StatusInt                       CreateDimension(EditElementHandleR elem, int dimView);
	BentleyStatus                   CreateDimension(EditElementHandleR dimElem, DPoint3dCR point);
	void                            ProjectPointsTo2dIfNeeded(DPoint3d   *points, int numPoints);
	bvector<DPoint3d>               GetSegmentPoints(HitPathCP hitPath);
	void                            FinishDimension(DgnButtonEventCR ev);
	bool                            ValidateSelection(HitPathCP hitPath);
	bool                            InsertPoints(EditElementHandleR dimElem, bvector<DPoint3d> points);
public:

	static      void                InstallNewInstance();
};//end
/*=================================================================================**//**
* An object of this type is required to supply necessary information about
* the dimension-style, text-style,rotational-matrix, symbology, view-number
* to the CreateDimensionElement(...) method of DimensionHandler class which
* uses this information to create dimension element.
*
* @bsiclass                                                               Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct      DimCreateData : IDimCreateData
{
	DimensionStylePtr               m_dimStyle;
	DgnTextStylePtr                 m_textStyle;
	RotMatrix                       m_dimRMatrix;
	RotMatrix                       m_viewRMatrix;
	Symbology                       m_symbology;
	int                             m_viewNumber;

	DimCreateData() { }
	DimCreateData(RotMatrixCR rMatrix, int vn, int alignment);

	DimensionStyleCR                _GetDimStyle()     const override { return *m_dimStyle; }
	DgnTextStyleCR                  _GetTextStyle()    const override { return *m_textStyle; }
	Symbology                       _GetSymbology()    const override { return m_symbology; }
	LevelId                         _GetLevelID()      const override { return LEVEL_DEFAULT_LEVEL_ID; }
	RotMatrixCR                     _GetDimRMatrix()   const override { return m_dimRMatrix; }
	RotMatrixCR                     _GetViewRMatrix()  const override { return m_viewRMatrix; }
	int                             _GetViewNumber()   const override { return m_viewNumber; }
};//end
