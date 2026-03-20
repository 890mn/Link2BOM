#include "DefaultDataSeeder.h"

#include "ArchiveController.h"
#include "ProjectController.h"
#include "CategoryController.h"
#include "BomTableModel.h"

DefaultDataSeeder::SeedData DefaultDataSeeder::seedData() const
{
    SeedData data;
    data.defaultProjectName = QStringLiteral("Default Project");
    data.headers = {
        QStringLiteral("\u9879\u76ee"),
        QStringLiteral("\u5546\u54c1\u7f16\u53f7"),
        QStringLiteral("\u54c1\u724c"),
        QStringLiteral("\u5382\u5bb6\u578b\u53f7"),
        QStringLiteral("\u5c01\u88c5"),
        QStringLiteral("\u5546\u54c1\u540d\u79f0"),
        QStringLiteral("\u8ba2\u8d2d\u6570\u91cf\uff08\u4fee\u6539\u540e\uff09"),
        QStringLiteral("\u5546\u54c1\u5355\u4ef7"),
        QStringLiteral("\u5546\u54c1\u91d1\u989d")
    };
    data.rows = {
        {QStringLiteral("Default Project"), QStringLiteral("C25804"), QStringLiteral("Yageo"), QStringLiteral("RC0603FR-0710KL"), QStringLiteral("0603"), QStringLiteral("Resistor 10K"), QStringLiteral("8"), QStringLiteral("0.0015"), QStringLiteral("0.0120")},
        {QStringLiteral("Default Project"), QStringLiteral("C14663"), QStringLiteral("Samsung"), QStringLiteral("CL10B104KB8NNNC"), QStringLiteral("0603"), QStringLiteral("Cap 100nF"), QStringLiteral("4"), QStringLiteral("0.0020"), QStringLiteral("0.0080")},
        {QStringLiteral("Default Project"), QStringLiteral("C21120"), QStringLiteral("Murata"), QStringLiteral("GRM188R71C105KA12D"), QStringLiteral("0603"), QStringLiteral("Cap 1uF"), QStringLiteral("3"), QStringLiteral("0.0061"), QStringLiteral("0.0183")},
        {QStringLiteral("Default Project"), QStringLiteral("C529431"), QStringLiteral("ST"), QStringLiteral("STM32G071KBT6"), QStringLiteral("LQFP32"), QStringLiteral("MCU"), QStringLiteral("1"), QStringLiteral("1.8200"), QStringLiteral("1.8200")},
        {QStringLiteral("Default Project"), QStringLiteral("C16581"), QStringLiteral("WCH"), QStringLiteral("CH340C"), QStringLiteral("SOP16"), QStringLiteral("USB-UART"), QStringLiteral("1"), QStringLiteral("0.3100"), QStringLiteral("0.3100")},
        {QStringLiteral("Default Project"), QStringLiteral("C29294"), QStringLiteral("TI"), QStringLiteral("TPS54331DR"), QStringLiteral("SOIC8"), QStringLiteral("DC-DC"), QStringLiteral("1"), QStringLiteral("0.7800"), QStringLiteral("0.7800")},
        {QStringLiteral("Default Project"), QStringLiteral("C5446"), QStringLiteral("Omron"), QStringLiteral("B3F-1000"), QStringLiteral("THT"), QStringLiteral("Tact Switch"), QStringLiteral("2"), QStringLiteral("0.0900"), QStringLiteral("0.1800")},
        {QStringLiteral("Default Project"), QStringLiteral("C7213"), QStringLiteral("Littelfuse"), QStringLiteral("1206L050"), QStringLiteral("1206"), QStringLiteral("PTC Fuse"), QStringLiteral("1"), QStringLiteral("0.1500"), QStringLiteral("0.1500")}
    };
    return data;
}

void DefaultDataSeeder::initialize(ArchiveController &archive,
                                   ProjectController &projects,
                                   CategoryController &categories,
                                   BomTableModel &model) const
{
    const SeedData data = seedData();
    archive.ensureDefaultSlots(data.headers,
                               data.rows,
                               projects.projectNames(true),
                               categories.categoryNames(),
                               data.defaultProjectName);

    const bool loaded = archive.loadSlot(0);
    if (!loaded) {
        model.setSourceData(data.headers, data.rows);
    }
}
