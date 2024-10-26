#pragma once
#ifndef GEODESY_CORE_UTIL_VARIABLE_H
#define GEODESY_CORE_UTIL_VARIABLE_H

#include <string>
#include <vector>
#include <ostream>

#include "../math.h"

namespace geodesy::core::util {

	class variable;

	class type {
	public:

		friend class variable;

		enum class id {
			UNKNOWN = -1,
			// Composite Structure Type
			STRUCT,
			// Number Types
			UCHAR,
			USHORT,
			UINT,
			CHAR,
			SHORT,
			INT,
			//HALF,
			FLOAT,
			DOUBLE,
			// Vector Types
			UCHAR2,
			UCHAR3,
			UCHAR4,
			USHORT2,
			USHORT3,
			USHORT4,
			UINT2,
			UINT3,
			UINT4,
			CHAR2,
			CHAR3,
			CHAR4,
			SHORT2,
			SHORT3,
			SHORT4,
			INT2,
			INT3,
			INT4,
			//HALF2,
			//HALF3,
			//HALF4,
			FLOAT2,
			FLOAT3,
			FLOAT4,
			// Matrix Types
			FLOAT2X2,
			FLOAT2X3,
			FLOAT2X4,
			FLOAT3X2,
			FLOAT3X3,
			FLOAT3X4,
			FLOAT4X2,
			FLOAT4X3,
			FLOAT4X4
		};

		static id id_of_string(const char* aTypeName);
		static const char* name_of(id aID);
		static size_t size_of(id aID);

		id							ID;
		std::vector<variable> 		Member;
		std::string					Name;

		type();
		type(type::id aID, const char* aName = "", std::vector<variable> aMemberList = std::vector<variable>(0));
		type(const type& aInput);
		type(type&& aInput) noexcept;
		~type();

		type& operator=(type::id aID);
		type& operator=(const type& aRhs);
		type& operator=(type&& aRhs) noexcept;

		size_t size() const;
		std::string str(int aDepth = 0) const;

		void clear();

	private:

		variable*					Master;

		void zero_out();

	};

	class variable {
	public:

		friend class type;

		type						Type;
		std::string					Name;
		std::vector<int> 			Dimension;

		variable();
		variable(type aType, const char* aName = "", std::vector<int> aDimensionList = std::vector<int>(0));
		variable(const variable& aInput);
		variable(variable&& aInput) noexcept;
		~variable();

		variable& operator=(const variable& aRhs);
		variable& operator=(variable&& aRhs) noexcept;

		variable& operator[](int aIndex);
		variable& operator[](const char* aFieldName);

		size_t size() const;
		size_t offset() const;
		std::string str(int aDepth = 0) const;

		void clear();

	private:

		variable*					Root;
		variable*					Parent;

		void zero_out();

	};

	std::ostream& operator<<(std::ostream& aOutputStream, const variable& aInput);

}

#endif // !GEODESY_CORE_UTIL_VARIABLE_H
