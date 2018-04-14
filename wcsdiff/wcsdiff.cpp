#include "stdafx.h"

using namespace std;

enum class wcs_diff : int { same, removed, child_changed, added };

bool print_diff(wcs_diff diff) {
	switch (diff) {
	case wcs_diff::same: /* printf_s("= "); return true;*/ return false;
	case wcs_diff::removed: printf_s("- "); return true;
	case wcs_diff::child_changed: printf_s("| "); return true;
	case wcs_diff::added: printf_s("+ "); return true;
	default: printf_s("? "); return true;
	}
}

struct wcs_diff_elem {
	wcs_diff diff;
	wstring name;
	wcs_diff_elem() : diff(wcs_diff::same) {}
	wcs_diff_elem(wcs_diff _diff, const wstring& _name) : diff(_diff), name(_name) {}
};

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

struct wcs_diff_component : wcs_diff_elem {
	vector<wcs_diff_elem> files;
	wcs_diff_component(wcs_diff _diff, const wstring& _name) : wcs_diff_elem(_diff, _name) {}
};

int _tmain(int argc, _TCHAR* argv[])
{
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

	const auto new_components = find_files_wcs(new_folder);
	const auto old_components = find_files_wcs(old_folder);

	printf_s(" new folder: %S%S\n", new_folder.c_str(), !new_components.empty() ? L"" : L" (EMPTY!)");
	printf_s(" old folder: %S%S\n", old_folder.c_str(), !old_components.empty() ? L"" : L" (EMPTY!)");

	printf_s("\n");

	printf_s(" diff legends: +: added, -: removed, |: files changed\n");
	printf_s("\n");

	vector<wcs_diff_component> component_diffs;

	for (const auto& new_component : new_components) {
		auto& new_files = new_component.second;
		wcs_diff_component diff(wcs_diff::same, new_component.first);
		auto old_component = old_components.find(new_component.first);
		if (old_component == old_components.end()) { // not found in old, so its new, and all members are new
			diff.diff = wcs_diff::added;
			for (auto& new_file : new_component.second) {
				diff.files.emplace_back(wcs_diff::added, new_file);
			}
			component_diffs.emplace_back(move(diff));
		} else {
			auto& old_files = old_component->second;
			for (auto& new_file : new_files) {
				if (find_if(begin(old_files), end(old_files), [&](const wstring& s) {
					return _wcsicmp(s.c_str(), new_file.c_str()) == 0;
				}) == old_files.end()) {
					diff.files.emplace_back(wcs_diff::added, new_file);
				}
			}
			for (auto& old_file : old_files) {
				if (find_if(begin(new_files), end(new_files), [&](const wstring& s) {
					return _wcsicmp(s.c_str(), old_file.c_str()) == 0;
				}) == new_files.end()) {
					diff.files.emplace_back(wcs_diff::removed, old_file);
				}
			}
			if (!diff.files.empty()) {
				diff.diff = wcs_diff::child_changed;
				component_diffs.emplace_back(move(diff));
			}
		}
	}

	for (const auto& old_component : old_components) {
		if (new_components.find(old_component.first) == new_components.end()) {
			wcs_diff_component diff(wcs_diff::removed, old_component.first);
			for (auto& old_file : old_component.second) {
				diff.files.emplace_back(wcs_diff::removed, old_file);
			}
			component_diffs.push_back(move(diff));
		}
	}

//	sort(begin(component_diffs), end(component_diffs));

	if (component_diffs.empty()) {
		printf_s(" found no differences.\n");
	} else {
		for (const auto& diff : component_diffs) {
			if (!print_diff(diff.diff)) continue;

			printf_s("%S", diff.name.c_str());

			printf_s(" (\n");
			for (const auto& f : diff.files) {
				if (!print_diff(f.diff)) continue;

				printf_s("\t%S\n", f.name.c_str());
			}

			print_diff(diff.diff);
			printf_s(")\n");

			printf_s("\n");
		}
	}

	return 0;
}
