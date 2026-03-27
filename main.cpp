#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/utsname.h>
#include <unistd.h>
#include <vector>
#include <cmath>

// Minimal color palette
namespace Colors {
const std::string RESET = "\033[0m";
const std::string BOLD = "\033[1m";
const std::string DIM = "\033[2m";
const std::string CYAN = "\033[36m";
const std::string WHITE = "\033[37m";
const std::string BRIGHT_WHITE = "\033[97m";
} // namespace Colors

std::string readFileLine(const std::string &filename, int lineNumber) {
	std::ifstream file(filename);
	std::string line;
	int currentLine = 0;

	while (std::getline(file, line)) {
		currentLine++;
		if (currentLine == lineNumber) {
			return line;
		}
	}
	return "";
}

std::string getFileContent(const std::string &filename) {
	std::ifstream file(filename);
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::string getOSName() {
	std::string content = getFileContent("/etc/os-release");
	std::istringstream stream(content);
	std::string line;

	while (std::getline(stream, line)) {
		if (line.find("PRETTY_NAME=") == 0) {
			size_t start = line.find('"') + 1;
			size_t end = line.rfind('"');
			if (start > 0 && end > start)
				return line.substr(start, end - start);
		}
		if (line.find("NAME=") == 0) {
			size_t start = line.find('"') + 1;
			size_t end = line.rfind('"');
			if (start > 0 && end > start)
				return line.substr(start, end - start);
		}
	}

	std::string osLine = readFileLine("/etc/os-release", 1);
	if (!osLine.empty() && osLine.find('=') != std::string::npos) {
		size_t start = osLine.find('"') + 1;
		size_t end = osLine.rfind('"');
		if (start > 0 && end > start)
			return osLine.substr(start, end - start);
	}

	return "Unknown";
}

std::string getKernelVersion() {
	struct utsname buffer;
	if (uname(&buffer) == 0) {
		return std::string(buffer.release);
	}
	return "Unknown";
}

std::string getHostname() {
	struct utsname buffer;
	if (uname(&buffer) == 0) {
		return std::string(buffer.nodename);
	}
	return "Unknown";
}

std::string getUptime() {
	std::ifstream uptime("/proc/uptime");
	long long uptime_seconds;
	char dot;
	long long frac;
	if (uptime >> uptime_seconds >> dot >> frac) {
		long long days = uptime_seconds / 86400;
		long long hours = (uptime_seconds % 86400) / 3600;
		long long minutes = (uptime_seconds % 3600) / 60;

		std::ostringstream result;
		if (days > 0)
			result << days << " day" << (days > 1 ? "s" : "") << ", ";
		if (hours > 0 || days > 0)
			result << hours << " hour" << (hours != 1 ? "s" : "") << ", ";
		result << minutes << " minute" << (minutes != 1 ? "s" : "");
		return result.str();
	}
	return "Unknown";
}

int countPackages() {
	int count = 0;

	// Debian/Ubuntu
	std::ifstream dpkg("/var/lib/dpkg/status");
	if (dpkg.good()) {
		std::string line;
		while (std::getline(dpkg, line)) {
			if (line.find("Package:") == 0)
				count++;
		}
		if (count > 0)
			return count;
	}

	// Arch
	DIR *pacman = opendir("/var/lib/pacman/local");
	if (pacman) {
		struct dirent *entry;
		while ((entry = readdir(pacman)) != NULL) {
			if (entry->d_name[0] != '.') {
				count++;
			}
		}
		closedir(pacman);
		if (count > 0)
			return count - 2; // Subtract . and ..
	}

	// NixOS
	DIR *nix = opendir("/nix/store");
	if (nix) {
		struct dirent *entry;
		while ((entry = readdir(nix)) != NULL) {
			if (entry->d_name[0] != '.')
				count++;
		}
		closedir(nix);
		if (count > 0)
			return count / 2;
	}

	return count;
}

std::string getShell() {
	const char *shell = getenv("SHELL");
	if (shell) {
		std::string shellPath(shell);
		size_t lastSlash = shellPath.rfind('/');
		if (lastSlash != std::string::npos)
			return shellPath.substr(lastSlash + 1);
		return shellPath;
	}
	return "Unknown";
}

std::string getTerminal() {
	const char *term = getenv("TERM");
	const char *termProgram = getenv("TERM_PROGRAM");

	if (termProgram) {
		std::string tp(termProgram);
		if (tp.find("vscode") != std::string::npos)
			return "VS Code Terminal";
		if (tp.find("Apple_Terminal") != std::string::npos)
			return "Terminal.app";
		if (tp.find("Hyper") != std::string::npos)
			return "Hyper";
	}

	if (term) {
		std::string t(term);
		if (t.find("xterm") != std::string::npos)
			return "Xterm";
		if (t.find("screen") != std::string::npos)
			return "Screen/Tmux";
		if (t.find("alacritty") != std::string::npos)
			return "Alacritty";
		if (t.find("kitty") != std::string::npos)
			return "Kitty";
		if (t.find("wezterm") != std::string::npos)
			return "WezTerm";
	}

	const char *colorterm = getenv("COLORTERM");
	if (colorterm) {
		std::string ct(colorterm);
		if (ct.find("gnome") != std::string::npos)
			return "GNOME Terminal";
		if (ct.find("konsole") != std::string::npos)
			return "Konsole";
	}

	return term ? std::string(term) : "Unknown";
}

std::string shortenGPU(std::string gpu) {
	// Try to find content in brackets, usually the most descriptive
	size_t lastOpen = gpu.rfind('[');
	size_t lastClose = gpu.rfind(']');
	if (lastOpen != std::string::npos && lastClose != std::string::npos && lastClose > lastOpen) {
		gpu = gpu.substr(lastOpen + 1, lastClose - lastOpen - 1);
	}

	// If it contains a slash, it often lists multiple names for the same chip, take the first
	size_t slash = gpu.find('/');
	if (slash != std::string::npos) {
		gpu = gpu.substr(0, slash);
	}

	// Remove common suffixes
	std::vector<std::string> removeList = {
		"Corporation",
		"Graphics Controller",
		"Integrated Graphics Controller",
		"Series",
		"LHR",
		"Lite Hash Rate"
	};

	for (const auto& s : removeList) {
		size_t pos = std::string::npos;
		while ((pos = gpu.find(s)) != std::string::npos) {
			gpu.erase(pos, s.length());
		}
	}

	// Remove "(rev ...)"
	size_t rev = gpu.find("(rev");
	if (rev != std::string::npos) {
		gpu = gpu.substr(0, rev);
	}

	// Clean up extra spaces
	gpu.erase(0, gpu.find_first_not_of(" \t"));
	gpu.erase(gpu.find_last_not_of(" \t\n\r") + 1);

	return gpu;
}

std::string shortenCPU(std::string cpu) {
	std::vector<std::string> removeList = {
		"with Radeon Graphics",
		"with Intel Graphics",
		"Processor",
		"CPU",
		" @ ",
		"(TM)",
		"(R)",
		"  "
	};

	for (const auto& s : removeList) {
		size_t pos = std::string::npos;
		while ((pos = cpu.find(s)) != std::string::npos) {
			cpu.erase(pos, s.length());
		}
	}

	// Remove extra core info if it's too long
	size_t paren = cpu.find('(');
	if (paren != std::string::npos && cpu.length() > 30) {
		cpu = cpu.substr(0, paren);
	}

	cpu.erase(0, cpu.find_first_not_of(" \t"));
	cpu.erase(cpu.find_last_not_of(" \t") + 1);

	return cpu;
}

std::string getGPU() {
	std::string gpu = "Unknown";

	FILE *pipe = popen("lspci 2>/dev/null | grep -i vga | head -n 1 | cut "
					   "-d':' -f3- | sed 's/^[ \\t]*//'",
					   "r");
	if (pipe) {
		char buffer[256];
		if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
			gpu = buffer;
			gpu.erase(gpu.find_last_not_of("\n\r") + 1);
		}
		pclose(pipe);
	}

	if (gpu == "Unknown" || gpu.empty()) {
		std::ifstream render("/sys/class/drm/renderD128/device/vendor");
		if (render.good()) {
			std::string vendor;
			render >> vendor;
			if (vendor == "0x10de") gpu = "NVIDIA GPU";
			else if (vendor == "0x8086") gpu = "Intel GPU";
			else if (vendor == "0x1002") gpu = "AMD GPU";
		}
	}

	return shortenGPU(gpu);
}

std::string getCPU() {
	std::ifstream cpuinfo("/proc/cpuinfo");
	std::string line;
	std::string modelName;
	int coreCount = 0;

	while (std::getline(cpuinfo, line)) {
		if (line.find("model name") == 0) {
			size_t colon = line.find(':');
			if (colon != std::string::npos) {
				modelName = line.substr(colon + 1);
				modelName.erase(0, modelName.find_first_not_of(" \t"));
				coreCount++;
			}
		}
	}

	if (!modelName.empty()) {
		return shortenCPU(modelName) + " (" + std::to_string(coreCount) + ")";
	}

	return "Unknown";
}

std::string getMemory() {
	std::ifstream meminfo("/proc/meminfo");
	std::string line;
	long long total = 0, available = 0;

	while (std::getline(meminfo, line)) {
		if (line.find("MemTotal:") == 0) {
			std::istringstream iss(line);
			std::string label;
			iss >> label >> total;
		} else if (line.find("MemAvailable:") == 0) {
			std::istringstream iss(line);
			std::string label;
			iss >> label >> available;
		}
	}

	if (total > 0) {
		long long used = total - available;
		double usedGB = used / 1024.0 / 1024.0;
		double totalGB = total / 1024.0 / 1024.0;
		int percentage = (used * 100) / total;

		std::ostringstream result;
		result << std::fixed << std::setprecision(1) << usedGB << " GiB / "
			   << std::setprecision(1) << totalGB << " GiB (" << percentage
			   << "%)";
		return result.str();
	}

	return "Unknown";
}

int getMemoryPercentage() {
	std::ifstream meminfo("/proc/meminfo");
	std::string line;
	long long total = 0, available = 0;

	while (std::getline(meminfo, line)) {
		if (line.find("MemTotal:") == 0) {
			std::istringstream iss(line);
			std::string label;
			iss >> label >> total;
		} else if (line.find("MemAvailable:") == 0) {
			std::istringstream iss(line);
			std::string label;
			iss >> label >> available;
		}
	}

	if (total > 0) {
		long long used = total - available;
		return (used * 100) / total;
	}
	return 0;
}

std::string getResolution() {
	FILE *pipe = popen("xrandr 2>/dev/null | grep ' connected' | head -n 1 | "
					   "grep -oP '\\d+x\\d+' | head -n 1",
					   "r");
	if (pipe) {
		char buffer[64];
		if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
			std::string res(buffer);
			res.erase(res.find_last_not_of("\n\r") + 1);
			pclose(pipe);
			if (!res.empty())
				return res;
		}
		pclose(pipe);
	}

	const char *width = getenv("COLUMNS");
	const char *height = getenv("LINES");
	if (width && height) {
		return std::string(width) + "x" + std::string(height) + " (terminal)";
	}

	return "Unknown";
}

std::string getWM() {
	const char *wm = getenv("XDG_CURRENT_DESKTOP");
	if (wm)
		return std::string(wm);

	wm = getenv("DESKTOP_SESSION");
	if (wm)
		return std::string(wm);

	FILE *pipe = popen("wmctrl -m 2>/dev/null | grep 'Name:' | cut -d':' -f2- "
					   "| sed 's/^[ \\t]*//'",
					   "r");
	if (pipe) {
		char buffer[256];
		if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
			std::string result(buffer);
			result.erase(result.find_last_not_of("\n\r") + 1);
			pclose(pipe);
			if (!result.empty())
				return result;
		}
		pclose(pipe);
	}

	return "Unknown";
}

// Simple ASCII art logo
std::string getAsciiArt() {
	return R"(
    ╔════════════════════════════╗
    ║   I N F I N I T Y          ║
    ║   S Y S T E M S            ║
    ╚════════════════════════════╝
   )";
}

// Simple separator
void printSeparator() {
	std::cout << "    " << Colors::DIM << "──────────────────────────────────────────────────" << Colors::RESET << std::endl;
}

int main() {
	// Gather all system information
	std::string os = getOSName();
	std::string kernel = getKernelVersion();
	std::string hostname = getHostname();
	std::string uptime = getUptime();
	int packages = countPackages();
	std::string shell = getShell();
	std::string terminal = getTerminal();
	std::string cpu = getCPU();
	std::string gpu = getGPU();
	std::string memory = getMemory();
	std::string wm = getWM();
	int memoryPercent = getMemoryPercentage();

	const char* user = getenv("USER");
	std::string username = user ? user : "user";

	std::cout << std::endl;

	// Minimal Header
	std::cout << "    " << Colors::CYAN << Colors::BOLD << username << Colors::RESET 
			  << Colors::DIM << "@" << Colors::RESET 
			  << Colors::CYAN << Colors::BOLD << hostname << Colors::RESET << std::endl;
	
	std::cout << "    " << Colors::DIM << "────────────────────────────────────────────" << Colors::RESET << std::endl;

	// Info rows with fixed label width
	auto printRow = [](const std::string& label, const std::string& value) {
		std::cout << "    " << Colors::CYAN << std::left << std::setw(12) << label << Colors::RESET 
				  << Colors::DIM << "│ " << Colors::RESET << value << std::endl;
	};

	printRow("OS", os);
	printRow("Kernel", kernel);
	printRow("Uptime", uptime);
	printRow("Packages", std::to_string(packages));
	printRow("Shell", shell);
	printRow("Terminal", terminal);
	printRow("WM", wm);
	printRow("CPU", cpu);
	printRow("GPU", gpu);
	printRow("Memory", memory);

	// Memory Bar
	std::cout << "    " << Colors::DIM << "────────────┴───────────────────────────────" << Colors::RESET << std::endl;
	std::cout << "    " << Colors::CYAN << std::left << std::setw(12) << "Usage" << Colors::RESET << Colors::DIM << "│ " << Colors::RESET;
	
	int barWidth = 30;
	int filled = (memoryPercent * barWidth) / 100;
	for (int i = 0; i < filled; i++) std::cout << Colors::CYAN << "■" << Colors::RESET;
	for (int i = 0; i < (barWidth - filled); i++) std::cout << Colors::DIM << "·" << Colors::RESET;
	std::cout << " " << Colors::BOLD << memoryPercent << "%" << Colors::RESET << std::endl;

	std::cout << std::endl;
	return 0;
}
