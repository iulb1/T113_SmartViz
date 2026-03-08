import os
from pathlib import Path

def merge_text_files(root_dir, output_file):
    """
    合并指定目录下所有 .md, .c, .h, .cpp, .txt 文件
    """
    extensions = ['.md', '.c', '.h', '.cpp', '.txt']
    all_files = []

    # 递归查找所有匹配的文件
    for ext in extensions:
        for file_path in Path(root_dir).rglob(f'*{ext}'):
            all_files.append(file_path)

    # 排序：先按扩展名分组，再按完整路径排序
    all_files.sort(key=lambda x: (x.suffix, str(x)))

    # 写入合并后的文件
    with open(output_file, 'w', encoding='utf-8') as out_f:
        out_f.write('=' * 80 + '\n')
        out_f.write(f'文件合并报告\n')
        out_f.write(f'合并时间: {os.path.basename(__file__)}\n')
        out_f.write(f'总计文件数: {len(all_files)}\n')
        out_f.write('=' * 80 + '\n\n')

        current_ext = None
        file_count = 0

        for file_path in all_files:
            # 添加文件类型分隔符
            if file_path.suffix != current_ext:
                current_ext = file_path.suffix
                out_f.write('\n' + '#' * 80 + '\n')
                out_f.write(f'# {current_ext.upper()} 文件 ({sum(1 for f in all_files if f.suffix == current_ext)} 个)\n')
                out_f.write('#' * 80 + '\n\n')

            # 添加单个文件头
            rel_path = file_path.relative_to(root_dir)
            out_f.write('-' * 80 + '\n')
            out_f.write(f'文件: {rel_path}\n')
            out_f.write('-' * 80 + '\n\n')

            # 读取并写入文件内容
            try:
                with open(file_path, 'r', encoding='utf-8') as in_f:
                    content = in_f.read()
                    out_f.write(content)
                    if not content.endswith('\n'):
                        out_f.write('\n')
                    out_f.write('\n')
                file_count += 1
            except UnicodeDecodeError:
                out_f.write('[无法读取此文件: 编码问题]\n\n')
                file_count += 1
            except Exception as e:
                out_f.write(f'[读取错误: {e}]\n\n')

        # 写入汇总信息
        out_f.write('\n\n' + '=' * 80 + '\n')
        out_f.write(f'合并完成\n')
        out_f.write(f'成功处理文件数: {file_count}/{len(all_files)}\n')
        out_f.write('=' * 80 + '\n')

if __name__ == '__main__':
    # 设置根目录和输出文件
    root_directory = r'E:\简历\上官\5. 随课开发文档'
    output_merged_file = os.path.join(root_directory, '所有文件合并.txt')

    print(f'开始合并文件...')
    print(f'根目录: {root_directory}')
    print(f'输出文件: {output_merged_file}')
    print()

    merge_text_files(root_directory, output_merged_file)

    print(f'合并完成!')
    print(f'合并后的文件保存在: {output_merged_file}')