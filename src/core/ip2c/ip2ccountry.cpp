//------------------------------------------------------------------------------
// ip2ccountry.cpp
//------------------------------------------------------------------------------
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA
//
//------------------------------------------------------------------------------
// Copyright (C) 2021 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "ip2ccountry.h"

#include <QObject>

IP2CCountry::IP2CCountry()
	:flag(nullptr)
{}

IP2CCountry::IP2CCountry(QString name)
	:flag(nullptr), name(name)
{
}

IP2CCountry::IP2CCountry(const QPixmap *flag, QString name)
	:flag(flag), name(name)
{
}

bool IP2CCountry::isValid() const
{
	return !name.isEmpty();
}

namespace
{
class IP2CCountryTr : public QObject
{
	Q_OBJECT

public:
	static QHash<QString, IP2CCountry> all()
	{
		QHash<QString, IP2CCountry> map;
		map.insert("ABW", IP2CCountry(tr("Aruba")));
		map.insert("AFG", IP2CCountry(tr("Afghanistan")));
		map.insert("AGO", IP2CCountry(tr("Angola")));
		map.insert("AIA", IP2CCountry(tr("Anguilla")));
		map.insert("ALB", IP2CCountry(tr("Albania")));
		map.insert("AND", IP2CCountry(tr("Andorra")));
		map.insert("ANT", IP2CCountry(tr("Netherlands Antilles")));
		map.insert("ARE", IP2CCountry(tr("United Arab Emirates")));
		map.insert("ARG", IP2CCountry(tr("Argentina")));
		map.insert("ARM", IP2CCountry(tr("Armenia")));
		map.insert("ASM", IP2CCountry(tr("American Samoa")));
		map.insert("ATA", IP2CCountry(tr("Antarctica")));
		map.insert("ATF", IP2CCountry(tr("French Southern Territories")));
		map.insert("ATG", IP2CCountry(tr("Antigua and Barbuda")));
		map.insert("AUS", IP2CCountry(tr("Australia")));
		map.insert("AUT", IP2CCountry(tr("Austria")));
		map.insert("AZE", IP2CCountry(tr("Azerbaijan")));
		map.insert("BDI", IP2CCountry(tr("Burundi")));
		map.insert("BEL", IP2CCountry(tr("Belgium")));
		map.insert("BEN", IP2CCountry(tr("Benin")));
		map.insert("BFA", IP2CCountry(tr("Burkina Faso")));
		map.insert("BGD", IP2CCountry(tr("Bangladesh")));
		map.insert("BGR", IP2CCountry(tr("Bulgaria")));
		map.insert("BHR", IP2CCountry(tr("Bahrain")));
		map.insert("BHS", IP2CCountry(tr("Bahamas")));
		map.insert("BIH", IP2CCountry(tr("Bosnia and Herzegovina")));
		map.insert("BLR", IP2CCountry(tr("Belarus")));
		map.insert("BLZ", IP2CCountry(tr("Belize")));
		map.insert("BMU", IP2CCountry(tr("Bermuda")));
		map.insert("BOL", IP2CCountry(tr("Bolivia")));
		map.insert("BRA", IP2CCountry(tr("Brazil")));
		map.insert("BRB", IP2CCountry(tr("Barbados")));
		map.insert("BRN", IP2CCountry(tr("Brunei")));
		map.insert("BTN", IP2CCountry(tr("Bhutan")));
		map.insert("BVT", IP2CCountry(tr("Bouvet Island")));
		map.insert("BWA", IP2CCountry(tr("Botswana")));
		map.insert("CAF", IP2CCountry(tr("Central African Republic")));
		map.insert("CAN", IP2CCountry(tr("Canada")));
		map.insert("CCK", IP2CCountry(tr("Cocos (Keeling) Islands")));
		map.insert("CHE", IP2CCountry(tr("Switzerland")));
		map.insert("CHL", IP2CCountry(tr("Chile")));
		map.insert("CHN", IP2CCountry(tr("China")));
		map.insert("CIV", IP2CCountry(tr("Ivory Coast")));
		map.insert("CMR", IP2CCountry(tr("Cameroon")));
		map.insert("COD", IP2CCountry(tr("Congo, the Democratic Republic of the")));
		map.insert("COG", IP2CCountry(tr("Congo")));
		map.insert("COK", IP2CCountry(tr("Cook Islands")));
		map.insert("COL", IP2CCountry(tr("Colombia")));
		map.insert("COM", IP2CCountry(tr("Comoros")));
		map.insert("CPV", IP2CCountry(tr("Cape Verde")));
		map.insert("CRI", IP2CCountry(tr("Costa Rica")));
		map.insert("CUB", IP2CCountry(tr("Cuba")));
		map.insert("CXR", IP2CCountry(tr("Christmas Island")));
		map.insert("CYM", IP2CCountry(tr("Cayman Islands")));
		map.insert("CYP", IP2CCountry(tr("Cyprus")));
		map.insert("CZE", IP2CCountry(tr("Czech Republic")));
		map.insert("DEU", IP2CCountry(tr("Germany")));
		map.insert("DJI", IP2CCountry(tr("Djibouti")));
		map.insert("DMA", IP2CCountry(tr("Dominica")));
		map.insert("DNK", IP2CCountry(tr("Denmark")));
		map.insert("DOM", IP2CCountry(tr("Dominican Republic")));
		map.insert("DZA", IP2CCountry(tr("Algeria")));
		map.insert("ECU", IP2CCountry(tr("Ecuador")));
		map.insert("EGY", IP2CCountry(tr("Egypt")));
		map.insert("ERI", IP2CCountry(tr("Eritrea")));
		map.insert("ESH", IP2CCountry(tr("Western Sahara")));
		map.insert("ESP", IP2CCountry(tr("Spain")));
		map.insert("EST", IP2CCountry(tr("Estonia")));
		map.insert("ETH", IP2CCountry(tr("Ethiopia")));
		map.insert("EU", IP2CCountry(tr("European Union")));
		map.insert("FIN", IP2CCountry(tr("Finland")));
		map.insert("FJI", IP2CCountry(tr("Fiji")));
		map.insert("FLK", IP2CCountry(tr("Falkland Islands (Malvinas)")));
		map.insert("FRA", IP2CCountry(tr("France")));
		map.insert("FRO", IP2CCountry(tr("Faroe Islands")));
		map.insert("FSM", IP2CCountry(tr("Micronesia, Federated States of")));
		map.insert("GAB", IP2CCountry(tr("Gabon")));
		map.insert("GBR", IP2CCountry(tr("United Kingdom")));
		map.insert("GEO", IP2CCountry(tr("Georgia")));
		map.insert("GGY", IP2CCountry(tr("Guernsey")));
		map.insert("GHA", IP2CCountry(tr("Ghana")));
		map.insert("GIB", IP2CCountry(tr("Gibraltar")));
		map.insert("GIN", IP2CCountry(tr("Guinea")));
		map.insert("GLP", IP2CCountry(tr("Guadeloupe")));
		map.insert("GMB", IP2CCountry(tr("Gambia")));
		map.insert("GNB", IP2CCountry(tr("Guinea-Bissau")));
		map.insert("GNQ", IP2CCountry(tr("Equatorial Guinea")));
		map.insert("GRC", IP2CCountry(tr("Greece")));
		map.insert("GRD", IP2CCountry(tr("Grenada")));
		map.insert("GRL", IP2CCountry(tr("Greenland")));
		map.insert("GTM", IP2CCountry(tr("Guatemala")));
		map.insert("GUF", IP2CCountry(tr("French Guiana")));
		map.insert("GUM", IP2CCountry(tr("Guam")));
		map.insert("GUY", IP2CCountry(tr("Guyana")));
		map.insert("HKG", IP2CCountry(tr("Hong Kong")));
		map.insert("HMD", IP2CCountry(tr("Heard Island and McDonald Islands")));
		map.insert("HND", IP2CCountry(tr("Honduras")));
		map.insert("HRV", IP2CCountry(tr("Croatia")));
		map.insert("HTI", IP2CCountry(tr("Haiti")));
		map.insert("HUN", IP2CCountry(tr("Hungary")));
		map.insert("IDN", IP2CCountry(tr("Indonesia")));
		map.insert("IMN", IP2CCountry(tr("Isle of Man")));
		map.insert("IND", IP2CCountry(tr("India")));
		map.insert("IOT", IP2CCountry(tr("British Indian Ocean Territory")));
		map.insert("IRL", IP2CCountry(tr("Ireland")));
		map.insert("IRN", IP2CCountry(tr("Iran, Islamic Republic of")));
		map.insert("IRQ", IP2CCountry(tr("Iraq")));
		map.insert("ISL", IP2CCountry(tr("Iceland")));
		map.insert("ISR", IP2CCountry(tr("Israel")));
		map.insert("ITA", IP2CCountry(tr("Italy")));
		map.insert("JAM", IP2CCountry(tr("Jamaica")));
		map.insert("JEY", IP2CCountry(tr("Jersey")));
		map.insert("JOR", IP2CCountry(tr("Jordan")));
		map.insert("JPN", IP2CCountry(tr("Japan")));
		map.insert("KAZ", IP2CCountry(tr("Kazakhstan")));
		map.insert("KEN", IP2CCountry(tr("Kenya")));
		map.insert("KGZ", IP2CCountry(tr("Kyrgyzstan")));
		map.insert("KHM", IP2CCountry(tr("Cambodia")));
		map.insert("KIR", IP2CCountry(tr("Kiribati")));
		map.insert("KNA", IP2CCountry(tr("Saint Kitts and Nevis")));
		map.insert("KOR", IP2CCountry(tr("South Korea")));
		map.insert("KWT", IP2CCountry(tr("Kuwait")));
		map.insert("LAO", IP2CCountry(tr("Lao People's Democratic Republic")));
		map.insert("LBN", IP2CCountry(tr("Lebanon")));
		map.insert("LBR", IP2CCountry(tr("Liberia")));
		map.insert("LBY", IP2CCountry(tr("Libya")));
		map.insert("LCA", IP2CCountry(tr("Saint Lucia")));
		map.insert("LIE", IP2CCountry(tr("Liechtenstein")));
		map.insert("LKA", IP2CCountry(tr("Sri Lanka")));
		map.insert("LSO", IP2CCountry(tr("Lesotho")));
		map.insert("LTU", IP2CCountry(tr("Lithuania")));
		map.insert("LUX", IP2CCountry(tr("Luxembourg")));
		map.insert("LVA", IP2CCountry(tr("Latvia")));
		map.insert("MAC", IP2CCountry(tr("Macao")));
		map.insert("MAR", IP2CCountry(tr("Morocco")));
		map.insert("MCO", IP2CCountry(tr("Monaco")));
		map.insert("MDA", IP2CCountry(tr("Moldova, Republic of")));
		map.insert("MDG", IP2CCountry(tr("Madagascar")));
		map.insert("MDV", IP2CCountry(tr("Maldives")));
		map.insert("MEX", IP2CCountry(tr("Mexico")));
		map.insert("MHL", IP2CCountry(tr("Marshall Islands")));
		map.insert("MKD", IP2CCountry(tr("Macedonia, the former Yugoslav Republic of")));
		map.insert("MLI", IP2CCountry(tr("Mali")));
		map.insert("MLT", IP2CCountry(tr("Malta")));
		map.insert("MMR", IP2CCountry(tr("Myanmar")));
		map.insert("MNE", IP2CCountry(tr("Montenegro")));
		map.insert("MNG", IP2CCountry(tr("Mongolia")));
		map.insert("MNP", IP2CCountry(tr("Northern Mariana Islands")));
		map.insert("MOZ", IP2CCountry(tr("Mozambique")));
		map.insert("MRT", IP2CCountry(tr("Mauritania")));
		map.insert("MSR", IP2CCountry(tr("Montserrat")));
		map.insert("MTQ", IP2CCountry(tr("Martinique")));
		map.insert("MUS", IP2CCountry(tr("Mauritius")));
		map.insert("MWI", IP2CCountry(tr("Malawi")));
		map.insert("MYS", IP2CCountry(tr("Malaysia")));
		map.insert("MYT", IP2CCountry(tr("Mayotte")));
		map.insert("NAM", IP2CCountry(tr("Namibia")));
		map.insert("NCL", IP2CCountry(tr("New Caledonia")));
		map.insert("NER", IP2CCountry(tr("Niger")));
		map.insert("NFK", IP2CCountry(tr("Norfolk Island")));
		map.insert("NGA", IP2CCountry(tr("Nigeria")));
		map.insert("NIC", IP2CCountry(tr("Nicaragua")));
		map.insert("NIU", IP2CCountry(tr("Niue")));
		map.insert("NLD", IP2CCountry(tr("Netherlands")));
		map.insert("NOR", IP2CCountry(tr("Norway")));
		map.insert("NPL", IP2CCountry(tr("Nepal")));
		map.insert("NRU", IP2CCountry(tr("Nauru")));
		map.insert("NZL", IP2CCountry(tr("New Zealand")));
		map.insert("OMN", IP2CCountry(tr("Oman")));
		map.insert("PAK", IP2CCountry(tr("Pakistan")));
		map.insert("PAN", IP2CCountry(tr("Panama")));
		map.insert("PCN", IP2CCountry(tr("Pitcairn")));
		map.insert("PER", IP2CCountry(tr("Peru")));
		map.insert("PHL", IP2CCountry(tr("Philippines")));
		map.insert("PLW", IP2CCountry(tr("Palau")));
		map.insert("PNG", IP2CCountry(tr("Papua New Guinea")));
		map.insert("POL", IP2CCountry(tr("Poland")));
		map.insert("PRI", IP2CCountry(tr("Puerto Rico")));
		map.insert("PRK", IP2CCountry(tr("Korea, Democratic People's Republic of")));
		map.insert("PRT", IP2CCountry(tr("Portugal")));
		map.insert("PRY", IP2CCountry(tr("Paraguay")));
		map.insert("PSE", IP2CCountry(tr("Palestinian Territory, Occupied")));
		map.insert("PYF", IP2CCountry(tr("French Polynesia")));
		map.insert("QAT", IP2CCountry(tr("Qatar")));
		map.insert("REU", IP2CCountry(tr("Réunion")));
		map.insert("ROU", IP2CCountry(tr("Romania")));
		map.insert("RUS", IP2CCountry(tr("Russian Federation")));
		map.insert("RWA", IP2CCountry(tr("Rwanda")));
		map.insert("SAU", IP2CCountry(tr("Saudi Arabia")));
		map.insert("SDN", IP2CCountry(tr("Sudan")));
		map.insert("SEN", IP2CCountry(tr("Senegal")));
		map.insert("SGP", IP2CCountry(tr("Singapore")));
		map.insert("SGS", IP2CCountry(tr("South Georgia and the South Sandwich Islands")));
		map.insert("SHN", IP2CCountry(tr("Saint Helena, Ascension and Tristan da Cunha")));
		map.insert("SJM", IP2CCountry(tr("Svalbard and Jan Mayen")));
		map.insert("SLB", IP2CCountry(tr("Solomon Islands")));
		map.insert("SLE", IP2CCountry(tr("Sierra Leone")));
		map.insert("SLV", IP2CCountry(tr("El Salvador")));
		map.insert("SMR", IP2CCountry(tr("San Marino")));
		map.insert("SOM", IP2CCountry(tr("Somalia")));
		map.insert("SPM", IP2CCountry(tr("Saint Pierre and Miquelon")));
		map.insert("SRB", IP2CCountry(tr("Serbia")));
		map.insert("SSD", IP2CCountry(tr("South Sudan")));
		map.insert("STP", IP2CCountry(tr("Sao Tome and Principe")));
		map.insert("SUR", IP2CCountry(tr("Suriname")));
		map.insert("SVK", IP2CCountry(tr("Slovakia")));
		map.insert("SVN", IP2CCountry(tr("Slovenia")));
		map.insert("SWE", IP2CCountry(tr("Sweden")));
		map.insert("SWZ", IP2CCountry(tr("Swaziland")));
		map.insert("SYC", IP2CCountry(tr("Seychelles")));
		map.insert("SYR", IP2CCountry(tr("Syrian Arab Republic")));
		map.insert("TCA", IP2CCountry(tr("Turks and Caicos Islands")));
		map.insert("TCD", IP2CCountry(tr("Chad")));
		map.insert("TGO", IP2CCountry(tr("Togo")));
		map.insert("THA", IP2CCountry(tr("Thailand")));
		map.insert("TJK", IP2CCountry(tr("Tajikistan")));
		map.insert("TKL", IP2CCountry(tr("Tokelau")));
		map.insert("TKM", IP2CCountry(tr("Turkmenistan")));
		map.insert("TLS", IP2CCountry(tr("Timor-Leste")));
		map.insert("TON", IP2CCountry(tr("Tonga")));
		map.insert("TTO", IP2CCountry(tr("Trinidad and Tobago")));
		map.insert("TUN", IP2CCountry(tr("Tunisia")));
		map.insert("TUR", IP2CCountry(tr("Turkey")));
		map.insert("TUV", IP2CCountry(tr("Tuvalu")));
		map.insert("TWN", IP2CCountry(tr("Taiwan")));
		map.insert("TZA", IP2CCountry(tr("Tanzania, United Republic of")));
		map.insert("UGA", IP2CCountry(tr("Uganda")));
		map.insert("UKR", IP2CCountry(tr("Ukraine")));
		map.insert("UMI", IP2CCountry(tr("United States Minor Outlying Islands")));
		map.insert("URY", IP2CCountry(tr("Uruguay")));
		map.insert("USA", IP2CCountry(tr("United States")));
		map.insert("UZB", IP2CCountry(tr("Uzbekistan")));
		map.insert("VAT", IP2CCountry(tr("Holy See (Vatican City State)")));
		map.insert("VCT", IP2CCountry(tr("Saint Vincent and the Grenadines")));
		map.insert("VEN", IP2CCountry(tr("Venezuela")));
		map.insert("VGB", IP2CCountry(tr("Virgin Islands, British")));
		map.insert("VIR", IP2CCountry(tr("Virgin Islands, U.S.")));
		map.insert("VNM", IP2CCountry(tr("Vietnam")));
		map.insert("VUT", IP2CCountry(tr("Vanuatu")));
		map.insert("WLF", IP2CCountry(tr("Wallis and Futuna")));
		map.insert("WSM", IP2CCountry(tr("Samoa")));
		map.insert("YEM", IP2CCountry(tr("Yemen")));
		map.insert("ZAF", IP2CCountry(tr("South Africa")));
		map.insert("ZMB", IP2CCountry(tr("Zambia")));
		map.insert("ZWE", IP2CCountry(tr("Zimbabwe")));
		return map;
	}
};
}

QHash<QString, IP2CCountry> IP2CCountry::all()
{
	return IP2CCountryTr::all();
}

#include "ip2ccountry.moc"
