// io.cpp  -  implementations for io.hpp
#include "careroute/io.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace careroute {

static std::vector<int> parse_skills(const std::string& token) {
    std::vector<int> out;
    if (token == "-" || token.empty())
        return out;
    std::stringstream ss(token);
    std::string part;
    while (std::getline(ss, part, ','))
        if (!part.empty())
            out.push_back(std::stoi(part));
    return out;
}

Instance load_instance(const std::string& path) {
    std::ifstream in(path);
    if (!in)
        throw std::runtime_error("cannot open instance file: " + path);

    Instance inst;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream ss(line);
        std::string kind;
        ss >> kind;
        if (kind == "speed") {
            ss >> inst.speed_kmh;
        } else if (kind == "carer") {
            Carer c;
            std::string skills;
            ss >> c.id >> c.x >> c.y >> c.shift_start >> c.shift_end
               >> c.max_minutes >> skills;
            c.skills = parse_skills(skills);
            inst.carers.push_back(std::move(c));
        } else if (kind == "visit") {
            Visit v;
            ss >> v.id >> v.x >> v.y >> v.skill >> v.duration >> v.tw_start
               >> v.tw_end >> v.preferred_carer;
            inst.visits.push_back(std::move(v));
        }
        // unknown lines are ignored on purpose, so the format can grow
    }
    return inst;
}

std::vector<Stop> schedule_route(const Instance& inst, const Route& r) {
    std::vector<Stop> stops;
    if (r.carer < 0 || r.visits.empty())
        return stops;
    const Carer& c = inst.carers[r.carer];
    double t = c.shift_start;
    double px = c.x, py = c.y;
    for (int vi : r.visits) {
        const Visit& v = inst.visits[vi];
        const double tr = inst.travel_min(px, py, v.x, v.y);
        Stop s;
        s.visit = v.id;
        s.arrive = t + tr;
        s.start = std::max(s.arrive, static_cast<double>(v.tw_start));
        s.finish = s.start + v.duration;
        stops.push_back(s);
        t = s.finish;
        px = v.x;
        py = v.y;
    }
    return stops;
}

void write_solution(const std::string& path, const Instance& inst,
                    const Solution& sol) {
    std::ofstream out(path);
    if (!out)
        throw std::runtime_error("cannot write solution file: " + path);

    const Metrics& m = sol.metrics;
    out << "# CareRoute solution\n";
    out << "metric cost " << m.cost << "\n";
    out << "metric travel " << m.travel << "\n";
    out << "metric unassigned " << m.unassigned << "\n";
    out << "metric lateness " << m.lateness << "\n";
    out << "metric overtime " << m.overtime << "\n";
    out << "metric continuity " << m.continuity_violations << "\n";
    out << "metric balance " << m.balance << "\n";

    for (const Route& r : sol.routes) {
        const std::vector<Stop> stops = schedule_route(inst, r);
        for (const Stop& s : stops)
            out << "route " << inst.carers[r.carer].id << " " << s.visit << " "
                << s.start << " " << s.finish << "\n";
    }
    for (int vi : sol.unassigned)
        out << "unassigned " << inst.visits[vi].id << "\n";
}

}  // namespace careroute
