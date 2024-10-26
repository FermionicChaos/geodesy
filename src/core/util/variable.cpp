#include <geodesy/core/util/variable.h>

#include <string>
#include <vector>
#include <ostream>

#include <geodesy/core/math.h>

namespace geodesy::core::util {

	struct type_info {
		type::id TypeID;
		std::string TypeName;
		type::id DataTypeID;
		int ElementCount;
		int Rows;
		int Columns;
		size_t Size;		
	};

	// TODO: Maybe move this to a gconfig.h file so it can be shared with other objects?

	// static struct {
	// 	type::id TypeID;
	// 	std::string TypeName;
	// 	type::id DataTypeID;
	// 	int ElementCount;
	// 	int Rows;
	// 	int Columns;
	// 	size_t Size;
	// } BuiltInType[] = {
	static std::vector<type_info> TypeDatabase = {
		{	type::id::STRUCT		,		"struct"		,	type::id::STRUCT		,		0		  ,		0		,		0		,	0									},
		{	type::id::UCHAR			,		"uchar"			,	type::id::UCHAR			,		1		  ,		1		,		1		,	sizeof(uchar)						},
		{	type::id::USHORT		,		"ushort"		,	type::id::USHORT		,		1		  ,		1		,		1		,	sizeof(ushort)						},
		{	type::id::UINT			,		"uint"			,	type::id::UINT			,		1		  ,		1		,		1		,	sizeof(uint)						},
		{	type::id::CHAR			,		"byte"			,	type::id::CHAR			,		1		  ,		1		,		1		,	sizeof(char)						},
		{	type::id::SHORT			,		"short"			,	type::id::SHORT			,		1		  ,		1		,		1		,	sizeof(short)						},
		{	type::id::INT			,		"int"			,	type::id::INT			,		1		  ,		1		,		1		,	sizeof(int)							},
		//{	type::id::HALF			,		"half"			,	type::id::HALF			,		1		  ,		1		,		1		,	sizeof(half)						},
		{	type::id::FLOAT			,		"float"			,	type::id::FLOAT			,		1		  ,		1		,		1		,	sizeof(float)						},
		{	type::id::DOUBLE		,		"double"		,	type::id::DOUBLE		,		1		  ,		1		,		1		,	sizeof(double)						},
		{	type::id::UCHAR2		,		"ubyte2"		,	type::id::UCHAR			,		2		  ,		1		,		1		,	sizeof(math::vec<uchar, 2>)			},
		{	type::id::UCHAR3		,		"ubyte3"		,	type::id::UCHAR			,		3		  ,		1		,		1		,	sizeof(math::vec<uchar, 3>)			},
		{	type::id::UCHAR4		,		"ubyte4"		,	type::id::UCHAR			,		4		  ,		1		,		1		,	sizeof(math::vec<uchar, 4>)			},
		{	type::id::USHORT2		,		"ushort2"		,	type::id::USHORT		,		2		  ,		1		,		1		,	sizeof(math::vec<ushort, 2>)		},
		{	type::id::USHORT3		,		"ushort3"		,	type::id::USHORT		,		3		  ,		1		,		1		,	sizeof(math::vec<ushort, 3>)		},
		{	type::id::USHORT4		,		"ushort4"		,	type::id::USHORT		,		4		  ,		1		,		1		,	sizeof(math::vec<ushort, 4>)		},
		{	type::id::UINT2			,		"uint2"			,	type::id::UINT			,		2		  ,		1		,		1		,	sizeof(math::vec<uint, 2>)			},
		{	type::id::UINT3			,		"uint3"			,	type::id::UINT			,		3		  ,		1		,		1		,	sizeof(math::vec<uint, 3>)			},
		{	type::id::UINT4			,		"uint4"			,	type::id::UINT			,		4		  ,		1		,		1		,	sizeof(math::vec<uint, 4>)			},
		{	type::id::CHAR2			,		"byte2"			,	type::id::CHAR			,		2		  ,		1		,		1		,	sizeof(math::vec<char, 2>)			},
		{	type::id::CHAR3			,		"byte3"			,	type::id::CHAR			,		3		  ,		1		,		1		,	sizeof(math::vec<char, 3>)			},
		{	type::id::CHAR4			,		"byte4"			,	type::id::CHAR			,		4		  ,		1		,		1		,	sizeof(math::vec<char, 4>)			},
		{	type::id::SHORT2		,		"short2"		,	type::id::SHORT			,		2		  ,		1		,		1		,	sizeof(math::vec<short, 2>)			},
		{	type::id::SHORT3		,		"short3"		,	type::id::SHORT			,		3		  ,		1		,		1		,	sizeof(math::vec<short, 3>)			},
		{	type::id::SHORT4		,		"short4"		,	type::id::SHORT			,		4		  ,		1		,		1		,	sizeof(math::vec<short, 4>)			},
		{	type::id::INT2			,		"int2"			,	type::id::INT			,		2		  ,		1		,		1		,	sizeof(math::vec<int, 2>)			},
		{	type::id::INT3			,		"int3"			,	type::id::INT			,		3		  ,		1		,		1		,	sizeof(math::vec<int, 3>)			},
		{	type::id::INT4			,		"int4"			,	type::id::INT			,		4		  ,		1		,		1		,	sizeof(math::vec<int, 4>)			},
		//{	type::id::HALF2			,		"half2"			,	type::id::HALF			,		2		  ,		1		,		1		,	sizeof(math::vec2<half>)			},
		//{	type::id::HALF3			,		"half3"			,	type::id::HALF			,		3		  ,		1		,		1		,	sizeof(math::vec3<half>)			},
		//{	type::id::HALF4			,		"half4"			,	type::id::HALF			,		4		  ,		1		,		1		,	sizeof(math::vec4<half>)			},
		{	type::id::FLOAT2		,		"float2"		,	type::id::FLOAT			,		2		  ,		1		,		1		,	sizeof(math::vec<float, 2>)			},
		{	type::id::FLOAT3		,		"float3"		,	type::id::FLOAT			,		3		  ,		1		,		1		,	sizeof(math::vec<float, 3>)			},
		{	type::id::FLOAT4		,		"float4"		,	type::id::FLOAT			,		4		  ,		1		,		1		,	sizeof(math::vec<float, 4>)			},
		{	type::id::FLOAT2X2		,		"float2x2"		,	type::id::FLOAT			,		2*2		  ,		2		,		2		,	sizeof(math::mat<float, 2, 2>)		},
		{	type::id::FLOAT2X3		,		"float2x3"		,	type::id::FLOAT			,		2*3		  ,		3		,		2		,	sizeof(math::mat<float, 2, 3>)		},
		{	type::id::FLOAT2X4		,		"float2x4"		,	type::id::FLOAT			,		2*4		  ,		4		,		2		,	sizeof(math::mat<float, 2, 4>)		},
		{	type::id::FLOAT3X2		,		"float3x2"		,	type::id::FLOAT			,		3*2		  ,		2		,		3		,	sizeof(math::mat<float, 3, 2>)		},
		{	type::id::FLOAT3X3		,		"float3x3"		,	type::id::FLOAT			,		3*3		  ,		3		,		3		,	sizeof(math::mat<float, 3, 3>)		},
		{	type::id::FLOAT3X4		,		"float3x4"		,	type::id::FLOAT			,		3*4		  ,		4		,		3		,	sizeof(math::mat<float, 3, 4>)		},
		{	type::id::FLOAT4X2		,		"float4x2"		,	type::id::FLOAT			,		4*2		  ,		2		,		4		,	sizeof(math::mat<float, 4, 2>)		},
		{	type::id::FLOAT4X3		,		"float4x3"		,	type::id::FLOAT			,		4*3		  ,		3		,		4		,	sizeof(math::mat<float, 4, 3>)		},
		{	type::id::FLOAT4X4		,		"float4x4"		,	type::id::FLOAT			,		4*4		  ,		4		,		4		,	sizeof(math::mat<float, 4, 4>)		}
	};

	type::id type::id_of_string(const char* aTypeName) {
		for (const type_info& Info : TypeDatabase) {
			if (Info.TypeName == aTypeName) {
				return Info.TypeID;
			}
		}
		return id::UNKNOWN;
	}

	const char* type::name_of(id aID) {
		for (const type_info& Info : TypeDatabase) {
			if (Info.TypeID == aID) {
				return Info.TypeName.c_str();
			}
		}
		return "";
	}

	size_t type::size_of(id aID) {
		for (const type_info& Info : TypeDatabase) {
			if (Info.TypeID == aID) {
				return Info.Size;
			}
		}
		return 0;
	}

	type::type() {
		this->zero_out();
	}

	type::type(type::id aID, const char* aName, std::vector<variable> aMemberList) : type() {
		this->ID = aID;
		if (ID == type::id::STRUCT) {
			this->Name 		= aName;
			this->Member 	= aMemberList;
		} else {
			this->Name 		= type::name_of(ID);
		}
	}

	type::type(const type& aInput) : type() {
		*this = aInput;
	}

	type::type(type&& aInput) noexcept : type() {
		this->Master 		= nullptr;
		this->ID			= aInput.ID;
		this->Member		= aInput.Member;
		this->Name			= aInput.Name;
		aInput.zero_out();
	}

	type::~type() {
		this->clear();
	}

	type& type::operator=(const type& aRhs) {
		if (this == &aRhs) return *this;
		this->ID 			= aRhs.ID;
		this->Member 		= std::vector<variable>(aRhs.Member.size());
		for (size_t i = 0; i < aRhs.Member.size(); i++) {
			if (this->Master != nullptr) {
				this->Member[i].Root 	= this->Master->Root;
				this->Member[i].Parent 	= this->Master;
			}
			this->Member[i] = aRhs.Member[i];
		}
		this->Name 		= aRhs.Name;
		return *this;
	}

	type& type::operator=(type&& aRhs) noexcept {
		this->clear();
		this->ID			= aRhs.ID;
		this->Member		= aRhs.Member;
		this->Name			= aRhs.Name;
		aRhs.zero_out();
		return *this;
	}
	
	type& type::operator=(type::id aID) {
		*this = type(aID);
		return *this;
	}

	size_t type::size() const {
		size_t TotalSize = 0;
		if (ID == type::id::STRUCT) {
			for (const variable& Member : this->Member) {
				TotalSize += Member.size();
			}
		}
		else {
			TotalSize = type::size_of(ID);
		}
		return TotalSize;
	}

	std::string type::str(int aDepth) const {
		std::string String;
		if (ID == type::id::STRUCT) {
			String += "struct " + this->Name + " {\n";
			for (const variable& Member : this->Member) {
				String.append(Member.str(aDepth + 1));
			}
			String += '}';
			if (aDepth > 1) {
				String += ';';
			}
		}
		else {
			String = this->Name;
		}
		return String;
	}

	void type::clear() {
		this->Member.clear();
		this->zero_out();
	}

	void type::zero_out() {
		this->ID		= type::id::UNKNOWN;
		this->Name		= "";
	}

	variable::variable() {
		this->Root				= this;
		this->Parent			= nullptr;
		this->Type.Master 		= this;
		this->zero_out();
	}

	variable::variable(type aType, const char* aName, std::vector<int> aDimensionList) : variable() {
		this->Type				= aType;
		this->Name				= aName;
		this->Dimension			= aDimensionList;
	}

	variable::variable(const variable& aInput) : variable() {
		*this = aInput;
	}

	variable::variable(variable&& aInput) noexcept : variable() {
		this->Type				= aInput.Type;
		this->Name				= aInput.Name;
		this->Dimension			= aInput.Dimension;
		aInput.zero_out();
	}

	variable::~variable() {
		this->clear();
	}

	variable& variable::operator=(const variable& aRhs) {
		if (this == &aRhs) return *this;
		this->Type			= aRhs.Type;
		this->Name			= aRhs.Name;
		this->Dimension		= aRhs.Dimension;
		return *this;
	}

	variable& variable::operator=(variable&& aRhs) noexcept {
		this->clear();
		this->Type			= aRhs.Type;
		this->Name			= aRhs.Name;
		this->Dimension		= aRhs.Dimension;
		aRhs.zero_out();
		return *this;
	}

	variable& variable::operator[](int aIndex) {
		return this->Type.Member[aIndex];
	}

	variable& variable::operator[](const char* aFieldName) {
		for (variable& Member : this->Type.Member) {
			if (Member.Name == aFieldName) {
				return Member;
			}
		}
		throw std::out_of_range("Field name not found.");
	}

	size_t variable::size() const {
		size_t ArraySize = 0;
		for (size_t i = 0; i < Dimension.size(); i++){
			ArraySize *= Dimension[i];
		}
		return Type.size() * ArraySize;
	}

	size_t variable::offset() const {
		size_t Offset = 0;
		return Offset;
	}

	std::string variable::str(int aDepth) const {
		std::string String = "";
		for (int i = 0; i < aDepth; i++) {
			String += "  ";
		}
		String += this->Type.str(aDepth + 1) + " ";
		String += this->Name;
		for (int N : this->Dimension) {
			String += "[" + std::to_string(N) + "]";
		}
		String += ";\n";
		return String;
	}

	void variable::clear() {
		this->Dimension.clear();
		this->zero_out();
	}

	void variable::zero_out() {
		this->Type = type::id::UNKNOWN;
		this->Name = "";
	}

	std::ostream& operator<<(std::ostream& aOutputStream, const variable& aInput) {
		return aOutputStream << aInput.str();
	}

}