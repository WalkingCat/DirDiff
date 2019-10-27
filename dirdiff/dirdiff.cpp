#include "stdafx.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	printf_s("\n DirDiff v0.1 https://github.com/WalkingCat/DirDiff\n\n");
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
				[&](const wstring& file_name, const wstring * new_file, const wstring * old_file) {
					bool printed_file_name = false;
					auto print_file_name = [&](const wchar_t prefix) {
						if (!printed_file_name) {
							print_group_name(new_files ? old_files ? L'*' : L'+' : L'-');
							if (printed_previous_file_name) {
								fwprintf_s(out, L"\n");
							}
							fwprintf_s(out, L"   %lc %ls\n", prefix, file_name.c_str());
							//printed_previous_file_name = printed_file_name = true;
						}
					};

					if (new_file == nullptr) {
						print_file_name('-');
						return;
					}

					if (old_file == nullptr) {
						print_file_name('+');
					}

					//TODO: diff files
				}
			);

			if (printed_group_name)
				fwprintf_s(out, L" %lc )\n", printed_group_prefix);
		}
	);

	fwprintf_s(out, L"\n");

	return 0;
}
