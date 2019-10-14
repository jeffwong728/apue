cmake_minimum_required(VERSION 3.12.1)
project(wxsvg CXX)

set(WXSVG_SOURCES
    include/wxSVG/Animated.h
    include/wxSVG/CSSRule.h
    include/wxSVG/CSSStyleDeclaration.h
    include/wxSVG/CSSValue.h
    include/wxSVG/CSSValues.h
    include/wxSVG/Document.h
    include/wxSVG/DocumentCSS.h
    include/wxSVG/DocumentEvent.h
    include/wxSVG/Element.h
    include/wxSVG/ElementTimeControl.h
    include/wxSVG/EventTarget.h
    include/wxSVG/ExifHandler.h
    include/wxSVG/GetSVGDocument.h
    include/wxSVG/imagsvg.h
    #include/wxSVG/mediadec_ffmpeg.h
    include/wxSVG/NodeList.h
    include/wxSVG/NodeListCls.h
    include/wxSVG/RGBColor.h
    include/wxSVG/String_wxsvg.h
    include/wxSVG/svg.h
    include/wxSVG/SVGAElement.h
    include/wxSVG/SVGAltGlyphDefElement.h
    include/wxSVG/SVGAltGlyphElement.h
    include/wxSVG/SVGAltGlyphItemElement.h
    include/wxSVG/SVGAngle.h
    include/wxSVG/SVGAnimateColorElement.h
    include/wxSVG/SVGAnimatedAngle.h
    include/wxSVG/SVGAnimatedBoolean.h
    include/wxSVG/SVGAnimatedEnumeration.h
    include/wxSVG/SVGAnimatedInteger.h
    include/wxSVG/SVGAnimatedLength.h
    include/wxSVG/SVGAnimatedLengthList.h
    include/wxSVG/SVGAnimatedNumber.h
    include/wxSVG/SVGAnimatedNumberList.h
    include/wxSVG/SVGAnimatedPathData.h
    include/wxSVG/SVGAnimatedPoints.h
    include/wxSVG/SVGAnimatedPreserveAspectRatio.h
    include/wxSVG/SVGAnimatedRect.h
    include/wxSVG/SVGAnimatedString.h
    include/wxSVG/SVGAnimatedTransformList.h
    include/wxSVG/SVGAnimatedType.h
    include/wxSVG/SVGAnimateElement.h
    include/wxSVG/SVGAnimateMotionElement.h
    include/wxSVG/SVGAnimateTransformElement.h
    include/wxSVG/SVGAnimationElement.h
    include/wxSVG/SVGCanvas.h
    include/wxSVG/SVGCanvasItem.h
    include/wxSVG/SVGCircleElement.h
    include/wxSVG/SVGClipPathElement.h
    include/wxSVG/SVGColor.h
    include/wxSVG/SVGColorProfileElement.h
    include/wxSVG/SVGColorProfileRule.h
    include/wxSVG/SVGComponentTransferFunctionElement.h
    include/wxSVG/SVGCoordinates.h
    include/wxSVG/SVGCSSRule.h
    include/wxSVG/svgctrl.h
    include/wxSVG/SVGCursorElement.h
    include/wxSVG/SVGDefinitionSrcElement.h
    include/wxSVG/SVGDefsElement.h
    include/wxSVG/SVGDescElement.h
    include/wxSVG/SVGDocument.h
    include/wxSVG/SVGDTD.h
    include/wxSVG/SVGElement.h
    include/wxSVG/SVGElementInstance.h
    include/wxSVG/SVGElementInstanceList.h
    include/wxSVG/SVGEllipseElement.h
    include/wxSVG/SVGEvent.h
    include/wxSVG/SVGExternalResourcesRequired.h
    include/wxSVG/SVGFEBlendElement.h
    include/wxSVG/SVGFEColorMatrixElement.h
    include/wxSVG/SVGFEComponentTransferElement.h
    include/wxSVG/SVGFECompositeElement.h
    include/wxSVG/SVGFEConvolveMatrixElement.h
    include/wxSVG/SVGFEDiffuseLightingElement.h
    include/wxSVG/SVGFEDisplacementMapElement.h
    include/wxSVG/SVGFEDistantLightElement.h
    include/wxSVG/SVGFEFloodElement.h
    include/wxSVG/SVGFEFuncAElement.h
    include/wxSVG/SVGFEFuncBElement.h
    include/wxSVG/SVGFEFuncGElement.h
    include/wxSVG/SVGFEFuncRElement.h
    include/wxSVG/SVGFEGaussianBlurElement.h
    include/wxSVG/SVGFEImageElement.h
    include/wxSVG/SVGFEMergeElement.h
    include/wxSVG/SVGFEMergeNodeElement.h
    include/wxSVG/SVGFEMorphologyElement.h
    include/wxSVG/SVGFEOffsetElement.h
    include/wxSVG/SVGFEPointLightElement.h
    include/wxSVG/SVGFESpecularLightingElement.h
    include/wxSVG/SVGFESpotLightElement.h
    include/wxSVG/SVGFETileElement.h
    include/wxSVG/SVGFETurbulenceElement.h
    include/wxSVG/SVGFilterElement.h
    include/wxSVG/SVGFilterPrimitiveStandardAttributes.h
    include/wxSVG/SVGFitToViewBox.h
    include/wxSVG/SVGFontElement.h
    include/wxSVG/SVGFontFaceElement.h
    include/wxSVG/SVGFontFaceFormatElement.h
    include/wxSVG/SVGFontFaceNameElement.h
    include/wxSVG/SVGFontFaceSrcElement.h
    include/wxSVG/SVGFontFaceUriElement.h
    include/wxSVG/SVGForeignObjectElement.h
    include/wxSVG/SVGGElement.h
    include/wxSVG/SVGGlyphElement.h
    include/wxSVG/SVGGlyphRefElement.h
    include/wxSVG/SVGGradientElement.h
    include/wxSVG/SVGHKernElement.h
    include/wxSVG/SVGICCColor.h
    include/wxSVG/SVGImageElement.h
    include/wxSVG/SVGLangSpace.h
    include/wxSVG/SVGLength.h
    include/wxSVG/SVGLengthCalculate.h
    include/wxSVG/SVGLengthList.h
    include/wxSVG/SVGLinearGradientElement.h
    include/wxSVG/SVGLineElement.h
    include/wxSVG/SVGLocatable.h
    include/wxSVG/SVGMarkerElement.h
    include/wxSVG/SVGMaskElement.h
    include/wxSVG/SVGMatrix.h
    include/wxSVG/SVGMetadataElement.h
    include/wxSVG/SVGMissingGlyphElement.h
    include/wxSVG/SVGMPathElement.h
    include/wxSVG/SVGNumber.h
    include/wxSVG/SVGNumberList.h
    include/wxSVG/SVGPaint.h
    include/wxSVG/SVGPathElement.h
    include/wxSVG/SVGPathSeg.h
    include/wxSVG/SVGPathSegArcAbs.h
    include/wxSVG/SVGPathSegArcRel.h
    include/wxSVG/SVGPathSegClosePath.h
    include/wxSVG/SVGPathSegCurvetoCubicAbs.h
    include/wxSVG/SVGPathSegCurvetoCubicRel.h
    include/wxSVG/SVGPathSegCurvetoCubicSmoothAbs.h
    include/wxSVG/SVGPathSegCurvetoCubicSmoothRel.h
    include/wxSVG/SVGPathSegCurvetoQuadraticAbs.h
    include/wxSVG/SVGPathSegCurvetoQuadraticRel.h
    include/wxSVG/SVGPathSegCurvetoQuadraticSmoothAbs.h
    include/wxSVG/SVGPathSegCurvetoQuadraticSmoothRel.h
    include/wxSVG/SVGPathSegLinetoAbs.h
    include/wxSVG/SVGPathSegLinetoHorizontalAbs.h
    include/wxSVG/SVGPathSegLinetoHorizontalRel.h
    include/wxSVG/SVGPathSegLinetoRel.h
    include/wxSVG/SVGPathSegLinetoVerticalAbs.h
    include/wxSVG/SVGPathSegLinetoVerticalRel.h
    include/wxSVG/SVGPathSegList.h
    include/wxSVG/SVGPathSegMovetoAbs.h
    include/wxSVG/SVGPathSegMovetoRel.h
    include/wxSVG/SVGPatternElement.h
    include/wxSVG/SVGPoint.h
    include/wxSVG/SVGPointList.h
    include/wxSVG/SVGPolygonElement.h
    include/wxSVG/SVGPolylineElement.h
    include/wxSVG/SVGPreserveAspectRatio.h
    include/wxSVG/SVGRadialGradientElement.h
    include/wxSVG/SVGRect.h
    include/wxSVG/SVGRectElement.h
    include/wxSVG/SVGRenderingIntent.h
    include/wxSVG/SVGScriptElement.h
    include/wxSVG/SVGSetElement.h
    include/wxSVG/SVGStopElement.h
    include/wxSVG/SVGStringList.h
    include/wxSVG/SVGStylable.h
    include/wxSVG/SVGStyleElement.h
    include/wxSVG/SVGSVGElement.h
    include/wxSVG/SVGSwitchElement.h
    include/wxSVG/SVGSymbolElement.h
    include/wxSVG/SVGTBreakElement.h
    include/wxSVG/SVGTests.h
    include/wxSVG/SVGTextContentElement.h
    include/wxSVG/SVGTextElement.h
    include/wxSVG/SVGTextPathElement.h
    include/wxSVG/SVGTextPositioningElement.h
    include/wxSVG/SVGTitleElement.h
    include/wxSVG/SVGTransform.h
    include/wxSVG/SVGTransformable.h
    include/wxSVG/SVGTransformList.h
    include/wxSVG/SVGTRefElement.h
    include/wxSVG/SVGTSpanElement.h
    include/wxSVG/SVGUnitTypes.h
    include/wxSVG/SVGURIReference.h
    include/wxSVG/SVGUseElement.h
    include/wxSVG/SVGVideoElement.h
    include/wxSVG/SVGViewElement.h
    include/wxSVG/SVGViewSpec.h
    include/wxSVG/SVGVKernElement.h
    include/wxSVG/SVGZoomAndPan.h
    include/wxSVG/SVGZoomEvent.h
    include/wxSVG/UIEvent.h
    include/wxSVG/ViewCSS.h
    include/wxSVGXML/svgxml.h
    include/wxSVGXML/svgxmlhelpr.h
    resources/pause.png.h
    resources/start.png.h
    resources/stop.png.h
    resources/wxsvg.png.h
    src/CSSStyleDeclaration.cpp
    src/CSSValue.cpp
    #src/css_colors.cpp
    #src/css_properties.cpp
    #src/css_values.cpp
    src/Elements_CopyConstructors.cpp
    src/Elements_GetAttribute.cpp
    src/Elements_GetAttributes.cpp
    src/Elements_HasAttribute.cpp
    src/Elements_SetAttribute.cpp
    src/ExifHandler.cpp
    src/GetSVGDocument.cpp
    src/imagsvg.cpp
    #src/mediadec_ffmpeg.cpp
    src/NodeList.cpp
    src/SVGAngle.cpp
    src/SVGAnimatedType.cpp
    src/SVGAnimateMotionElement.cpp
    src/SVGAnimateTransformElement.cpp
    src/SVGAnimationElement.cpp
    src/SVGCanvas.cpp
    src/SVGCanvasItem.cpp
    src/SVGCircleElement.cpp
    src/SVGColor.cpp
    src/svgctrl.cpp
    src/SVGDocument.cpp
    #src/SVGDocument_CreateElement.cpp
    src/SVGElementInstanceList.cpp
    src/SVGEllipseElement.cpp
    src/SVGFEGaussianBlurElement.cpp
    src/SVGFilterElement.cpp
    src/SVGFitToViewBox.cpp
    src/SVGImageElement.cpp
    src/SVGLength.cpp
    src/SVGLengthList.cpp
    src/SVGLineElement.cpp
    src/SVGLocatable.cpp
    src/SVGMarkerElement.cpp
    src/SVGMatrix.cpp
    src/SVGNumberList.cpp
    src/SVGPaint.cpp
    src/SVGPathElement.cpp
    src/SVGPathSegList.cpp
    src/SVGPoint.cpp
    src/SVGPointList.cpp
    src/SVGPolygonElement.cpp
    src/SVGPolylineElement.cpp
    src/SVGPreserveAspectRatio.cpp
    src/SVGRadialGradientElement.cpp
    src/SVGRect.cpp
    src/SVGRectElement.cpp
    src/SVGStringList.cpp
    src/SVGStylable.cpp
    src/SVGSVGElement.cpp
    src/SVGTests.cpp
    src/SVGTextContentElement.cpp
    src/SVGTextElement.cpp
    src/SVGTransform.cpp
    src/SVGTransformable.cpp
    src/SVGTransformList.cpp
    src/SVGUseElement.cpp
    src/SVGVideoElement.cpp
    src/cairo/SVGCanvasCairo.cpp
    src/cairo/SVGCanvasCairo.h
    src/cairo/SVGCanvasImageCairo.cpp
    src/cairo/SVGCanvasImageCairo.h
    src/cairo/SVGCanvasPathCairo.cpp
    src/cairo/SVGCanvasPathCairo.h
    src/cairo/SVGCanvasTextCairo.cpp
    src/cairo/SVGCanvasTextCairo.h
    #src/skia/Gradient.cpp
    #src/skia/Gradient.h
    #src/skia/SVGCanvasPathSkia.cpp
    #src/skia/SVGCanvasPathSkia.h
    #src/skia/SVGCanvasSkia.cpp
    #src/skia/SVGCanvasSkia.h
    src/svgxml/svgxml.cpp
    src/svgxml/svgxmlhelpr.cpp
    svgview/svgview.cpp
    svgview/svgview.h
)

set(CMAKE_DEBUG_POSTFIX "d")

find_package(EXPAT REQUIRED)
find_package(unofficial-cairo CONFIG REQUIRED)
find_package(wxWidgets REQUIRED COMPONENTS base core adv aui html media xrc gl qa richtext stc ribbon propgrid webview)

if(MSVC)
    set(pdb_output_dir "${CMAKE_CURRENT_BINARY_DIR}/pdb-files")
    set(CMAKE_PDB_OUTPUT_DIRECTORY "${pdb_output_dir}")
    set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY "${pdb_output_dir}")
    
    get_cmake_property(is_multi GENERATOR_IS_MULTI_CONFIG)
    if(is_multi)
        set(config_suffix "$<CONFIG>")
    else()
        set(config_suffix "")
    endif()

    include(GNUInstallDirs)
    if(BUILD_SHARED_LIBS)
        set(pdb_dst ${CMAKE_INSTALL_BINDIR})
    else()
        set(pdb_dst ${CMAKE_INSTALL_LIBDIR})
    endif()

    install(DIRECTORY "${pdb_output_dir}/${config_suffix}/" DESTINATION ${pdb_dst})
endif()

add_library(wxsvg ${WXSVG_SOURCES})
set_target_properties(wxsvg PROPERTIES ARCHIVE_OUTPUT_NAME wxsvg)
target_compile_options(wxsvg PRIVATE "/MP")
get_target_property(wxsvgBaseName wxsvg ARCHIVE_OUTPUT_NAME)
set_target_properties(wxsvg PROPERTIES COMPILE_PDB_NAME_DEBUG "${wxsvgBaseName}${CMAKE_DEBUG_POSTFIX}")

target_include_directories(wxsvg PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include/wxSVG")
target_include_directories(wxsvg PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include/wxSVGXML")
target_include_directories(wxsvg PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_include_directories(wxsvg PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_compile_definitions(wxsvg PRIVATE ${wxWidgets_DEFINITIONS})
target_compile_definitions(wxsvg PRIVATE $<$<CONFIG:Debug>:${wxWidgets_DEFINITIONS_DEBUG}>)
target_compile_options(wxsvg PRIVATE ${wxWidgets_CXX_FLAGS})
target_link_libraries(wxsvg PUBLIC ${wxWidgets_LIBRARIES})

set(libs)
list(APPEND libs EXPAT::EXPAT)
list(APPEND libs unofficial::cairo::cairo)
target_link_libraries(wxsvg PUBLIC ${libs})

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()

install(TARGETS wxsvg
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)

install(DIRECTORY include/ DESTINATION include OPTIONAL)
