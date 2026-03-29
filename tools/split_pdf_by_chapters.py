#!/usr/bin/env python3
"""
PDF按章节拆分工具
将《Mastering-the-FreeRTOS-Real-Time-Kernel》按章节拆分为markdown文档
"""

import os
import re
from pdfminer.high_level import extract_pages
from pdfminer.layout import LTTextContainer

# 配置参数
PDF_PATH = '/home/space/doc/Mastering-the-FreeRTOS-Real-Time-Kernel.v1.1.0.pdf'
OUTPUT_DIR = '/home/space/FreeRTOSDemo/doc/tutorial/Mastering-the-FreeRTOS-Kernel/'
CHAPTER_PATTERNS = [
    # 匹配章节标题模式
    r'^\d+\s+',
    r'^Appendix\s+[A-Z]',
]

# 章节映射（根据索引预先定义）
CHAPTER_MAPPING = [
    # 第一部分：入门
    {'pattern': r'^1\s', 'title': '第一章：前言', 'filename': '01_前言.md'},
    {'pattern': r'^2\s', 'title': '第二章：FreeRTOS内核分发', 'filename': '02_FreeRTOS内核分发.md'},
    {'pattern': r'^3\s', 'title': '第三章：堆内存管理', 'filename': '03_堆内存管理.md'},
    
    # 第二部分：核心特性
    {'pattern': r'^4\s', 'title': '第四章：任务管理', 'filename': '04_任务管理.md'},
    {'pattern': r'^5\s', 'title': '第五章：队列', 'filename': '05_队列.md'},
    {'pattern': r'^6\s', 'title': '第六章：软件定时器', 'filename': '06_软件定时器.md'},
    {'pattern': r'^7\s', 'title': '第七章：中断管理', 'filename': '07_中断管理.md'},
    {'pattern': r'^8\s', 'title': '第八章：资源管理', 'filename': '08_资源管理.md'},
    {'pattern': r'^9\s', 'title': '第九章：事件组', 'filename': '09_事件组.md'},
    
    # 第三部分：高级特性
    {'pattern': r'^10\s', 'title': '第十章：任务通知', 'filename': '10_任务通知.md'},
    {'pattern': r'^11\s', 'title': '第十一章：低功耗支持', 'filename': '11_低功耗支持.md'},
    {'pattern': r'^12\s', 'title': '第十二章：内存管理其他主题', 'filename': '12_内存管理其他主题.md'},
    {'pattern': r'^13\s', 'title': '第十三章：故障排查', 'filename': '13_故障排查.md'},
    
    # 附录 - 让我们找到它们再说
]

def extract_text_from_pdf(pdf_path):
    """从PDF提取所有文本内容，按页返回"""
    pages_text = []
    for page_layout in extract_pages(pdf_path):
        page_text = ''
        for element in page_layout:
            if isinstance(element, LTTextContainer):
                page_text += element.get_text()
        pages_text.append(page_text)
    return pages_text

def clean_text(text):
    """清理提取的文本"""
    # 移除页码
    text = re.sub(r'\n\s*\d+\s*\n', r'\n', text)
    # 移除页眉页脚常见内容
    text = re.sub(r'Mastering the FreeRTOS.*Kernel.*\d+', '', text)
    # 清理多余空行
    text = re.sub(r'\n\s*\n\s*\n', r'\n\n', text)
    return text.strip()

def find_chapter_start(page_text, chapter_info):
    """在页面中查找章节开头"""
    pattern = chapter_info['pattern']
    if re.search(pattern, page_text, re.IGNORECASE | re.MULTILINE):
        return True
    return False

def split_by_chapters(pages_text):
    """按章节拆分文本"""
    chapters = []
    processed_chapters = set()
    current_chapter = None
    current_text = ''
    
    # 遍历每一页
    for page_idx, page_text in enumerate(pages_text):
        cleaned_page = clean_text(page_text)
        
        # 检查是否是新章节
        found_new_chapter = False
        for chapter_info in CHAPTER_MAPPING:
            if chapter_info['filename'] not in processed_chapters and find_chapter_start(cleaned_page, chapter_info):
                # 如果已有当前章节，保存它
                if current_chapter is not None:
                    chapters.append({
                        'info': current_chapter,
                        'content': current_text
                    })
                # 开始新章节
                current_chapter = chapter_info
                processed_chapters.add(chapter_info['filename'])
                current_text = cleaned_page + '\n\n'
                found_new_chapter = True
                print(f"找到新章节: {chapter_info['title']} 在第{page_idx+1}页")
                break
        
        if not found_new_chapter and current_chapter is not None:
            current_text += cleaned_page + '\n\n'
    
    # 添加最后一个章节
    if current_chapter is not None and current_text:
        chapters.append({
            'info': current_chapter,
            'content': current_text
        })
        print(f"添加最后章节: {current_chapter['title']}")
    
    print(f"总共收集到 {len(chapters)} 个章节")
    return chapters

def save_chapters(chapters, output_dir):
    """保存章节到markdown文件"""
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    saved_files = []
    for chapter in chapters:
        filename = os.path.join(output_dir, chapter['info']['filename'])
        content = f'# {chapter["info"]["title"]}\n\n{chapter["content"]}'
        
        with open(filename, 'w', encoding='utf-8') as f:
            f.write(content)
        
        saved_files.append(filename)
        print(f'已保存: {filename}')
    
    return saved_files

def update_index(chapters, index_path):
    """更新章节索引文件，标记已完成提取"""
    if not os.path.exists(index_path):
        print(f'索引文件不存在: {index_path}')
        return
    
    with open(index_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 更新章节勾选状态
    for chapter in chapters:
        title = chapter['info']['title']
        filename = chapter['info']['filename']
        # 将 [ ] 改为 [x]
        pattern = re.compile(r'- \[ \] .*' + re.escape(title.split('：')[-1]))
        content = pattern.sub(f'- [x] [{title}]({filename})', content)
    
    with open(index_path, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f'索引已更新: {index_path}')

def main():
    """主函数"""
    print(f'开始处理PDF: {PDF_PATH}')
    
    if not os.path.exists(PDF_PATH):
        print(f'错误: PDF文件不存在: {PDF_PATH}')
        return
    
    # 检查依赖
    try:
        import pdfminer
    except ImportError:
        print('错误: 需要安装pdfminer.six: pip install pdfminer.six')
        return
    
    # 提取文本
    print('正在提取文本...')
    pages_text = extract_text_from_pdf(PDF_PATH)
    print(f'共提取 {len(pages_text)} 页')
    
    # 按章节拆分
    print('正在按章节拆分...')
    chapters = split_by_chapters(pages_text)
    print(f'找到 {len(chapters)} 个章节')
    
    # 保存文件
    print('正在保存文件...')
    saved_files = save_chapters(chapters, OUTPUT_DIR)
    
    # 更新索引
    index_path = os.path.join(OUTPUT_DIR, '章节索引.md')
    if os.path.exists(index_path):
        update_index(chapters, index_path)
    
    print('\n处理完成!')
    print(f'共保存 {len(saved_files)} 个文件:')
    for f in saved_files:
        print(f'  - {f}')

if __name__ == '__main__':
    main()
