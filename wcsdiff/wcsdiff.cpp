#include "stdafx.h"

using namespace std;

enum diff_options {
	diffNone		= 0x0,
	diffOld			= 0x1,
	diffNew			= 0x2,
	diffHelp = 0x80000000,
};

const struct { const wchar_t* arg; const wchar_t* arg_alt; const wchar_t* params_desc; const wchar_t* description; const diff_options options; } cmd_options[] = {
	{ L"?",		L"help",			nullptr,		L"show this help",						diffHelp },
	{ L"n",		L"new",				L"<filename>",	L"specify new file(s)",					diffNew },
	{ L"o",		L"old",				L"<filename>",	L"specify old file(s)",					diffOld },
};

void print_usage() {
	printf_s("\tUsage: wcsdiff [options]\n\n");
	for (auto o = std::begin(cmd_options); o != std::end(cmd_options); ++o) {
		if (o->arg != nullptr) printf_s("\t-%S", o->arg); else printf_s("\t");

		int len = 0;
		if (o->arg_alt != nullptr){
			len = wcslen(o->arg_alt);
			printf_s("\t--%S", o->arg_alt);
		} else printf_s("\t");

		if (len < 6) printf_s("\t");

		if(o->params_desc != nullptr) len += printf_s(" %S", o->params_desc);

		if (len < 14) printf_s("\t");

		printf_s("\t: %S\n", o->description);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	auto out = stdout;

	int options = diffNone;
	const wchar_t* err_arg = nullptr;
	wstring new_folder, old_folder;

	printf_s("\n WcsDiff v0.1 https://github.com/WalkingCat/WcsDiff\n\n");

	for(int i = 1; i < argc; ++i) {
		const wchar_t* arg = argv[i];
		if ((arg[0] == '-') || ((arg[0] == '/'))) {
			diff_options curent_option = diffNone;
			if ((arg[0] == '-') && (arg[1] == '-')) {
				for (auto o = std::begin(cmd_options); o != std::end(cmd_options); ++o) {
					if ((o->arg_alt != nullptr) &&(_wcsicmp(arg + 2, o->arg_alt) == 0)) { curent_option = o->options; }
				}
			} else {
				for (auto o = std::begin(cmd_options); o != std::end(cmd_options); ++o) {
					if ((o->arg != nullptr) && (_wcsicmp(arg + 1, o->arg) == 0)) { curent_option = o->options; }
				}
			}

			bool valid = false;
			if (curent_option != diffNone) {
				valid = true;
				if (curent_option == diffNew) {
					if ((i + 1) < argc) new_folder = argv[++i];
					else valid = false;
				} else if (curent_option == diffOld) {
					if ((i + 1) < argc) old_folder = argv[++i];
					else valid = false;
				} else options = (options | curent_option);
			} 
			if (!valid && (err_arg == nullptr)) err_arg = arg;
		} else { if (new_folder.empty()) new_folder = arg; else err_arg = arg; }
	}

	if ((new_folder.empty() && old_folder.empty()) || (err_arg != nullptr) || (options & diffHelp)) {
		if (err_arg != nullptr) printf_s("\tError in option: %S\n\n", err_arg);
		print_usage();
		return 0;
	}

	fwprintf_s(out, L" new folder: %ls", new_folder.c_str());
	auto new_file_groups = find_files_wcs(new_folder);
	fwprintf_s(out, L"%ls\n", !new_file_groups.empty() ? L"" : L" (EMPTY!)");

	fwprintf_s(out, L" old folder: %ls", old_folder.c_str());
	auto old_file_groups = find_files_wcs(old_folder);
	fwprintf_s(out, L"%ls\n", !old_file_groups.empty() ? L"" : L" (EMPTY!)");

	fwprintf_s(out, L"\n");

	if (new_file_groups.empty() & old_file_groups.empty()) return 0; // at least one of them must exists

	fwprintf_s(out, L" legends: +: added, -: removed, *: changed\n");

	const map<wstring, wstring> empty_files;
	diff_maps(new_file_groups, old_file_groups,
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
