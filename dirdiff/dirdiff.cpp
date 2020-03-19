#include "stdafx.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	printf_s("\n DirDiff v0.2 https://github.com/WalkingCat/DirDiff\n\n");
	const auto& params = init_diff_params(argc, argv);

	if (params.show_help || (!params.error.empty()) || (params.new_files_pattern.empty() && params.old_files_pattern.empty())) {
		if (!params.error.empty()) {
			printf_s("\t%ls\n\n", params.error.c_str());
		}
		if (params.show_help) print_cmdl_usage(L"dirdiff", diff_cmdl::options, diff_cmdl::default_option);
		return 0;
	}

	auto out = params.out;
	fwprintf_s(out, L"\n legends: +: added, -: removed, *: changed\n");

	const map<wstring, wstring> empty_files;
	diff_maps(params.new_file_groups, params.old_file_groups,
		[&](const wstring& group_name, const map<wstring, wstring>* new_files, const map<wstring, wstring>* old_files) {
			bool printed_group_name = false;
			wchar_t printed_group_prefix = L' ';
			auto print_group_name = [&](const wchar_t prefix) {
				if (!printed_group_name) {
					fwprintf_s(out, L"\n %lc %ls (\n", prefix, group_name.c_str());
					printed_group_name = true;
					printed_group_prefix = prefix;
				}
			};

			bool printed_previous_file_name = false;
			diff_maps(new_files ? *new_files : empty_files, old_files ? *old_files : empty_files,
				[&](const wstring& file_name, const wstring* new_file, const wstring* old_file) {
					bool printed_file_name = false;
					auto print_file_name = [&](const wchar_t prefix) {
						if (!printed_file_name) {
							print_group_name(new_files ? old_files ? L'*' : L'+' : L'-');
							if (printed_previous_file_name) {
								fwprintf_s(out, L"\n");
							}
							fwprintf_s(out, L"   %lc %ls\n", prefix, file_name.c_str());
							printed_previous_file_name = printed_file_name = true;
						}
					};

					if (new_file == nullptr) {
						print_file_name('-');
						return;
					}

					if (old_file == nullptr) {
						print_file_name('+');
						return;
					}

					static const std::set<wstring_view> text_file_exts = {
						L".adml",
						L".inf_loc",
					};

					const auto file_ext = tolower(filesystem::path(file_name).extension().wstring());
					if (text_file_exts.find(file_ext) != text_file_exts.end()) {
						const filesystem::path new_path(*new_file), old_path(*old_file);
						diff_sequences(read_text_file(*new_file), read_text_file(*old_file),
							[&](const wstring* new_line, const wstring* old_line) {
								if (new_line && old_line) {
									if (wcscmp(new_line->c_str(), old_line->c_str()) != 0) {
										print_file_name('*');
										fwprintf_s(out, L"     * %ls\n", new_line->c_str());
										fwprintf_s(out, L"     $ %ls\n", old_line->c_str());
									}
								} else if (new_line) {
									print_file_name('*');
									fwprintf_s(out, L"     + %ls\n", new_line->c_str());
								} else if (old_line) {
									print_file_name('*');
									fwprintf_s(out, L"     - %ls\n", old_line->c_str());
								}
							}
						);

						//if (filesystem::file_size(new_path) != filesystem::file_size(old_path)) {
						//	print_file_name('*');
						//	printf_s("     size: %I64u <-> %I64u\n", filesystem::file_size(new_path), filesystem::file_size(old_path));
						//	return;
						//}
					}
				}
			);

			if (printed_group_name)
				fwprintf_s(out, L" %lc )\n", printed_group_prefix);
		}
	);

	fwprintf_s(out, L"\n");

	return 0;
}
