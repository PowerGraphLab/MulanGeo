/**
 * @file DocumentExport.h
 * @brief Document 模块导出宏定义
 * @author hxxcxx
 * @date 2026-04-22
 */
#pragma once

#ifdef BUILDING_DOCUMENT
  #define DOCUMENT_API __declspec(dllexport)
#else
  #define DOCUMENT_API __declspec(dllimport)
#endif
