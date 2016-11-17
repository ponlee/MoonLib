#ifndef _CPP_CHARTDIR_H_
#define _CPP_CHARTDIR_H_

#include <stdint.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

class CppChartDir
{
public:

    /**
     *
     * @param   const string & path                     保存文件路径
     * @param   const string & title                    图表标题
     * @param   const vector<string> & xLabels          X坐标标签
     * @param   const vector<vector<double> > & datas   线条数据
     * @param   const vector<string> & dataNames        线条图例名称，长度必须>=datas的长度
     * @param   const uint32_t labelStep                横坐标标签跳过长度
     * @param   uint32_t width                          图表宽度
     * @param   uint32_t height                         图表高度
     * @retval  void
     * @author  moontan
     */
    static void MakeSplineLine(const string &path,
                               const string &title,
                               const vector<string> &xLabels,
                               const vector<vector<double> > &datas,
                               const vector<string> &dataNames,
                               const uint32_t labelStep,
                               uint32_t width, uint32_t height);

    static void MakeMultiAxes(const string &path,
                               const string &title,
                               const vector<string> &xLabels,
                               const vector<vector<double> > &datas,
                               const vector<string> &dataNames,
                               const uint32_t labelStep,
                               uint32_t width, uint32_t height);
};

#endif
