#include "CppChartDir.h"

#include <chartdir/chartdir.h>

#include "CppLog.h"
#include "CppSystem.h"

void CppChartDir::MakeSplineLine(const string &path,
                                 const string &title,
                                 const vector<string> &xLabels,
                                 const vector<vector<double> > &datas,
                                 const vector<string> &dataNames,
                                 const uint32_t labelStep,
                                 uint32_t width,
                                 uint32_t height)
{
    //     {
    // 
    //         c->addLegend(55, 32, false, "arialbd.ttf", 9)->setBackground(Chart::Transparent);
    // 
    //         // Add a title box to the chart using 15 pts Times Bold Italic font. The title is
    //         // in CDML and includes embedded images for highlight. The text is white (ffffff)
    //         // on a dark red (880000) background, with soft lighting effect from the right
    //         // side.
    //         c->addTitle(
    //             "<*block,valign=absmiddle*><*img=star.png*><*img=star.png*> Performance "
    //             "Enhancer <*img=star.png*><*img=star.png*><*/*>", "timesbi.ttf", 15, 0xffffff
    //             )->setBackground(0x880000, -1, Chart::softLighting(Chart::Right));
    // 
    //         // Add a title to the y axis
    //         c->yAxis()->setTitle("Energy Concentration (KJ per liter)");
    // 
    //         // Set the labels on the x axis
    //         c->xAxis()->setLabels(StringArray(labels, (int)(sizeof(labels) / sizeof(labels[0]
    //             ))));
    // 
    //         // Add a title to the x axis using CMDL
    //         c->xAxis()->setTitle(
    //             "<*block,valign=absmiddle*><*img=clock.png*>  Elapsed Time (hour)<*/*>");
    // 
    //         // Set the axes width to 2 pixels
    //         c->xAxis()->setWidth(2);
    //         c->yAxis()->setWidth(2);
    // 
    //         // Add a spline layer to the chart
    //         SplineLayer *layer = c->addSplineLayer();
    // 
    //         // Set the default line width to 2 pixels
    //         layer->setLineWidth(2);
    // 
    //         // Add a data set to the spline layer, using blue (0000c0) as the line color,
    //         // with yellow (ffff00) circle symbols.
    //         layer->addDataSet(DoubleArray(data1, (int)(sizeof(data1) / sizeof(data1[0]))),
    //                           0x0000c0, "Target Group")->setDataSymbol(Chart::CircleSymbol, 9, 0xffff00);
    // 
    //         // Add a data set to the spline layer, using brown (982810) as the line color,
    //         // with pink (f040f0) diamond symbols.
    //         layer->addDataSet(DoubleArray(data0, (int)(sizeof(data0) / sizeof(data0[0]))),
    //                           0x982810, "Control Group")->setDataSymbol(Chart::DiamondSymbol, 11, 0xf040f0)
    //                           ;
    // 
    //         // Add a custom CDML text at the bottom right of the plot area as the logo
    //         c->addText(575, 250,
    //                    "<*block,valign=absmiddle*><*img=small_molecule.png*> <*block*>"
    //                    "<*font=timesbi.ttf,size=10,color=804040*>Molecular\nEngineering<*/*>"
    //                    )->setAlignment(Chart::BottomRight);
    // 
    //         // Output the chart
    //         c->makeChart(path.c_str());
    // 
    //         //free up resources
    //         delete c;
    //         return;
    //     }

    if (datas.size() > dataNames.size())
    {
        THROW("datas.size[%u]>dataNames.size[%u]", datas.size(), dataNames.size());
    }

    // 常量区
    static uint32_t WIDTH_SPACE = 55;       // 宽度空白
    static uint32_t HEIGHT_SPACE = 45;      // 高度空白
    static uint32_t LEGEND_X = 45;          // 图例X
    static uint32_t LEGEND_Y = 17;          // 图例Y

    XYChart c(width, height);
    // 设置字体
    c.setDefaultFonts("simsun.ttc", "simsun.ttc", "simsun.ttc", "simsun.ttc");
    c.setRoundedFrame();

    // 添加绘图区
    c.setPlotArea(WIDTH_SPACE, HEIGHT_SPACE, width - WIDTH_SPACE * 2, height - HEIGHT_SPACE * 2,
                  0xffffff, -1, -1, 0xcccccc, 0xcccccc);

    // 添加图例
    c.addLegend(LEGEND_X, LEGEND_Y, false, NULL, 9)->setBackground(Chart::Transparent);

    // 图表标题
    c.addTitle(title.c_str());

    // X轴说明
    vector<const char *> labelsPtr;
    for (vector<string>::const_iterator label_it = xLabels.begin(); label_it != xLabels.end(); ++label_it)
    {
        labelsPtr.push_back(label_it->c_str());
    }
    c.xAxis()->setLabels(StringArray(labelsPtr.data(), labelsPtr.size()));
    c.xAxis()->setLabelStep(labelStep);

    // 加数据线条
    SplineLayer *layer = c.addSplineLayer();
    int32_t i = 0;
    for (vector<vector<double> >::const_iterator data_it = datas.begin(); data_it != datas.end(); ++data_it, ++i)
    {
        layer->addDataSet(DoubleArray(data_it->data(), data_it->size()), -1, dataNames[i].c_str());
    }

    c.makeChart(path.c_str());
    
    // 使用ImageMagic去除注册信息
    CppSystem::ExcuteCommand(CppString::GetArgs("convert '%s' -gravity South -chop 0x9 '%s'", path.c_str(), path.c_str()));
}

void CppChartDir::MakeMultiAxes(const string &path, const string &title, const vector<string> &xLabels, const vector<vector<double> > &datas, const vector<string> &dataNames, const uint32_t labelStep, uint32_t width, uint32_t height)
{
    if (datas.size() > dataNames.size())
    {
        THROW("datas.size[%u]>dataNames.size[%u]", datas.size(), dataNames.size());
    }

    // 常量区
    static uint32_t WIDTH_SPACE = 55;       // 宽度空白
    static uint32_t HEIGHT_SPACE = 55;      // 高度空白
    static uint32_t LEGEND_X = 45;          // 图例X
    static uint32_t LEGEND_Y = 17;          // 图例Y

    XYChart c(width, height);
    // 设置字体
    c.setDefaultFonts("simsun.ttc", "simsun.ttc", "simsun.ttc", "simsun.ttc");
    c.setRoundedFrame();

    // 添加绘图区
    c.setPlotArea(WIDTH_SPACE, HEIGHT_SPACE, width - WIDTH_SPACE * 2, height - HEIGHT_SPACE * 2,
                  0xffffff, -1, -1, 0xcccccc, 0xcccccc);

    // 添加图例
    c.addLegend(LEGEND_X, LEGEND_Y, false, NULL, 9)->setBackground(Chart::Transparent);

    // 图表标题
    c.addTitle(title.c_str());

    // X轴说明
    vector<const char *> labelsPtr;
    for (vector<string>::const_iterator label_it = xLabels.begin(); label_it != xLabels.end(); ++label_it)
    {
        labelsPtr.push_back(label_it->c_str());
    }
    c.xAxis()->setLabels(StringArray(labelsPtr.data(), labelsPtr.size()));
    c.xAxis()->setLabelStep(labelStep);

    // 加数据线条
    int32_t i = 0;
    for (vector<vector<double> >::const_iterator data_it = datas.begin(); data_it != datas.end(); ++data_it, ++i)
    {
        SplineLayer *layer = c.addSplineLayer();
        Axis *axis = c.addAxis(((i & 0x1) == 0) ? Chart::Left : Chart::Right, 10);
        axis->setTitle(dataNames[i].c_str())->setAlignment(((i & 0x1) == 0) ? Chart::TopLeft2 : Chart::TopRight2);
        layer->addDataSet(DoubleArray(data_it->data(), data_it->size()), -1, dataNames[i].c_str());
        layer->setUseYAxis(axis);
    }

    c.makeChart(path.c_str());

    // 使用ImageMagic去除注册信息
    CppSystem::ExcuteCommand(CppString::GetArgs("convert '%s' -gravity South -chop 0x9 '%s'", path.c_str(), path.c_str()));
}
