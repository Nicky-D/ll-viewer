/**
 * @file llobjectcostmanager.cpp
 * @brief Classes for managing object-related costs (rendering, streaming, etc) across multiple versions.
 *
 * $LicenseInfo:firstyear=2019&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2019, Linden Research, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"
#include "llobjectcostmanager.h"
#include "llvoavatar.h"
#include "llappearancemgr.h"
#include "llviewerobject.h"
#include "llvovolume.h"
#include "lldrawable.h"
#include "lldrawpool.h"
#include "llface.h"
#include "llpartdata.h"
#include "llviewerpartsource.h"
#include "llmeshrepository.h"
#include "llsd.h"

#include <set>

#define VALIDATE_COSTS

typedef std::set<LLUUID> texture_ids_t;

// ARC remove before release
#ifndef LL_RELEASE_FOR_DOWNLOAD
#ifdef MSVC
#pragma optimize("", off)
#endif
#endif

class LLPrimCostData
{
public:
	LLPrimCostData();
	~LLPrimCostData();

	void asLLSD( LLSD& sd ) const;

	// Face counts
	U32 m_alpha_mask_faces;
	U32 m_alpha_faces;
	U32 m_animtex_faces;
	U32 m_bumpmap_faces;
	U32 m_bump_any_faces;
	U32 m_flexi_vols;
	U32 m_full_bright_faces;
	U32 m_glow_faces;
	U32 m_invisi_faces;
	U32 m_materials_faces;
	U32 m_media_faces;
	U32 m_planar_faces;
	U32 m_shiny_faces;
	U32 m_shiny_any_faces;

	U32 m_normalmap_faces;
    U32 m_specmap_faces;

	// Volume counts
	U32 m_materials_vols;
	U32 m_mesh_vols;
	U32 m_weighted_mesh_vols; // rigged
	U32 m_particle_source_vols;
	U32 m_produces_light_vols;
	U32 m_sculpt_vols;

	// Other stats
	U32 m_num_particles;
	F32 m_part_size;
	U32 m_num_triangles_v1;
	U32 m_num_triangles_v2;
	bool m_is_animated_object;
	bool m_is_root_edit;

	// Triangle-related
	U32 m_triangle_count;
	U32 m_triangle_count_lowest;
	U32 m_triangle_count_low;
	U32 m_triangle_count_medium;
	U32 m_triangle_count_high;

	F32 m_actual_triangles_charged;
	
	// Texture ids
	texture_ids_t m_diffuse_ids;
	texture_ids_t m_sculpt_ids;
	texture_ids_t m_normal_ids;
	texture_ids_t m_specular_ids;
};

LLPrimCostData::LLPrimCostData():
	// Face counts
	m_alpha_mask_faces(0),
	m_alpha_faces(0),
	m_animtex_faces(0),
	m_bumpmap_faces(0),
	m_bump_any_faces(0),
	m_flexi_vols(0),
	m_full_bright_faces(0),
	m_glow_faces(0),
	m_invisi_faces(0),
	m_materials_faces(0),
	m_media_faces(0),
	m_planar_faces(0),
	m_shiny_faces(0),
	m_shiny_any_faces(0),
	m_normalmap_faces(0),
    m_specmap_faces(0),

	// Volume counts
	m_materials_vols(0),
	m_mesh_vols(0),
	m_weighted_mesh_vols(0),
	m_particle_source_vols(0),
	m_produces_light_vols(0),
	m_sculpt_vols(0),

	// Triangle
	m_triangle_count(0),
	m_triangle_count_lowest(0),
	m_triangle_count_low(0),
	m_triangle_count_medium(0),
	m_triangle_count_high(0),

	// Other stats
	m_num_particles(0),
	m_part_size(0.f),
	m_num_triangles_v1(0),
	m_num_triangles_v2(0),
	m_is_animated_object(false),
	m_is_root_edit(false)
{
}
	
LLPrimCostData::~LLPrimCostData()
{
}

void LLPrimCostData::asLLSD( LLSD& sd ) const
{
	sd["m_alpha_mask_faces"] = (LLSD::Integer) m_alpha_mask_faces;
	sd["m_alpha_faces"] = (LLSD::Integer) m_alpha_faces;
	sd["m_animtex_faces"] = (LLSD::Integer) m_animtex_faces;
	sd["m_bumpmap_faces"] = (LLSD::Integer) m_bumpmap_faces;
	sd["m_bump_any_faces"] = (LLSD::Integer) m_bump_any_faces;
	sd["m_flexi_vols"] = (LLSD::Integer) m_flexi_vols;
	sd["m_full_bright_faces"] = (LLSD::Integer) m_full_bright_faces;
	sd["m_glow_faces"] = (LLSD::Integer) m_glow_faces;
	sd["m_invisi_faces"] = (LLSD::Integer) m_invisi_faces;
	sd["m_materials_faces"] = (LLSD::Integer) m_materials_faces;
	sd["m_media_faces"] = (LLSD::Integer) m_media_faces;
	sd["m_planar_faces"] = (LLSD::Integer) m_planar_faces;
	sd["m_shiny_faces"] = (LLSD::Integer) m_shiny_faces;
	sd["m_shiny_any_faces"] = (LLSD::Integer) m_shiny_any_faces;
	sd["m_normalmap_faces"] = (LLSD::Integer) m_normalmap_faces;
	sd["m_specmap_faces"] = (LLSD::Integer) m_specmap_faces;
	sd["m_materials_vols"] = (LLSD::Integer) m_materials_vols;
	sd["m_mesh_vols"] = (LLSD::Integer) m_mesh_vols;
	sd["m_weighted_mesh_vols"] = (LLSD::Integer) m_weighted_mesh_vols;
	sd["m_particle_source_vols"] = (LLSD::Integer) m_particle_source_vols;
	sd["m_produces_light_vols"] = (LLSD::Integer) m_produces_light_vols;
	sd["m_sculpt_vols"] = (LLSD::Integer) m_sculpt_vols;
	sd["m_num_particles"] = (LLSD::Integer) m_num_particles;
	sd["m_part_size"] = (LLSD::Real) m_part_size;
	sd["m_num_triangles_v1"] = (LLSD::Integer) m_num_triangles_v1;
	sd["m_num_triangles_v2"] = (LLSD::Integer) m_num_triangles_v2;
	sd["m_is_animated_object"] = (LLSD::Boolean) m_is_animated_object;
	sd["m_is_root_edit"] = (LLSD::Boolean) m_is_root_edit;

	sd["m_triangle_count"] = (LLSD::Integer) m_triangle_count;
	sd["m_triangle_count_lowest("] = (LLSD::Integer) m_triangle_count_lowest;
	sd["m_triangle_count_low"] = (LLSD::Integer) m_triangle_count_low;
	sd["m_triangle_count_medium"] = (LLSD::Integer) m_triangle_count_medium;
	sd["m_triangle_count_high"] = (LLSD::Integer) m_triangle_count_high;
}

class LLObjectCostManagerImpl
{
  public:
	LLObjectCostManagerImpl() {}
	~LLObjectCostManagerImpl() {}

	F32 getStreamingCost(U32 version, const LLVOVolume *vol);
	F32 getStreamingCostV1(const LLVOVolume *vol);
	F32 getStreamingCostV2(const LLVOVolume *vol);

	F32 getRenderCost(U32 version, const LLVOVolume *vol);
	F32 getRenderCostV1(const LLVOVolume *vol);
	F32 getRenderCostV2(const LLVOVolume *vol);

	F32 getRenderCostLinkset(U32 version, const LLViewerObject *root);
	F32 getRenderCostLinksetV1(const LLViewerObject *root);
	F32 getRenderCostLinksetV2(const LLViewerObject *root);

	// Accumulate data for a single prim.
	void getPrimCostData(const LLVOVolume *vol, LLPrimCostData& cost_data);

	U32 textureCostsV1(const texture_ids_t& ids);
	U32 textureCostsV2(const texture_ids_t& all_sculpt_ids, const texture_ids_t& all_diffuse_ids,
					   const texture_ids_t& all_normal_ids, const texture_ids_t& all_specular_ids);
	U32 textureCostsV2(const texture_ids_t& ids, U32 multiplier = 1);

	F32 triangleCostsV1(LLPrimCostData& cost_data); 
	F32 triangleCostsV2(LLPrimCostData& cost_data); 
};

F32 LLObjectCostManagerImpl::getRenderCostLinkset(U32 version, const LLViewerObject *root)
{
	F32 render_cost = 0.f;

	if (version == 0)
	{
		version = LLObjectCostManager::instance().getCurrentCostVersion();
	}
	if (version == 1)
	{
		render_cost = getRenderCostLinksetV1(root);
#ifdef VALIDATE_COSTS
		// FIXME ARC need to rework legacy code to make comparison possible
#endif
	}
	else if (version == 2)
	{
		render_cost = getRenderCostLinksetV2(root);
	}
	else
	{
		LL_WARNS("Arctan") << "Unrecognized version " << version << LL_ENDL;
	}

	return render_cost;
}

typedef std::vector<const LLVOVolume*> const_vol_vec_t;

void get_volumes_for_linkset(const LLViewerObject *root, const_vol_vec_t& volumes)
{
	const LLVOVolume *root_vol = dynamic_cast<const LLVOVolume*>(root);
	if (root_vol && root_vol->isRootEdit())	
	{
		volumes.push_back(root_vol);
		
		LLViewerObject::const_child_list_t children = root_vol->getChildren();
		for (LLViewerObject::const_child_list_t::const_iterator child_iter = children.begin();
			 child_iter != children.end();
			 ++child_iter)
		{
			LLViewerObject* child_obj = *child_iter;
			LLVOVolume *child_vol = dynamic_cast<LLVOVolume*>( child_obj );
			if (child_vol)
			{
				volumes.push_back(child_vol);
			}
		}
	}
}

F32 LLObjectCostManagerImpl::getRenderCostLinksetV1(const LLViewerObject *root)
{
	F32 cost = 0.f;
	texture_ids_t all_sculpt_ids, all_diffuse_ids, all_normal_ids, all_specular_ids;

	const_vol_vec_t volumes;
	get_volumes_for_linkset(root,volumes);

	for (std::vector<const LLVOVolume*>::const_iterator it = volumes.begin();
		 it != volumes.end(); ++it)
	{
		const LLVOVolume *vol = *it;

		LLPrimCostData cost_data;
		getPrimCostData(vol, cost_data);

		// Charge for effective triangles
		cost += triangleCostsV1(cost_data);

		// Accumulate texture ids
		all_sculpt_ids.insert(cost_data.m_sculpt_ids.begin(), cost_data.m_sculpt_ids.end());
		all_diffuse_ids.insert(cost_data.m_diffuse_ids.begin(), cost_data.m_diffuse_ids.end());

		all_normal_ids.insert(cost_data.m_normal_ids.begin(), cost_data.m_normal_ids.end());
		all_specular_ids.insert(cost_data.m_specular_ids.begin(), cost_data.m_specular_ids.end());
	}
	
	// Material textures not included in V1 costs
	cost += textureCostsV1(all_sculpt_ids);
	cost += textureCostsV1(all_diffuse_ids);

	// Animated Object surcharge
	const F32 animated_object_surcharge = 1000;
	if (root->isAnimatedObject())
	{
		cost += animated_object_surcharge;
	}

	return cost;
}

F32 LLObjectCostManagerImpl::getRenderCostLinksetV2(const LLViewerObject *root)
{
	F32 cost = 0.f;
	texture_ids_t all_sculpt_ids, all_diffuse_ids, all_normal_ids, all_specular_ids;

	const_vol_vec_t volumes;
	get_volumes_for_linkset(root,volumes);

	for (std::vector<const LLVOVolume*>::const_iterator it = volumes.begin();
		 it != volumes.end(); ++it)
	{
		const LLVOVolume *vol = *it;

		LLPrimCostData cost_data;
		getPrimCostData(vol, cost_data);

		// Charge for effective triangles
		cost += triangleCostsV2(cost_data);

		// Accumulate texture ids
		all_sculpt_ids.insert(cost_data.m_sculpt_ids.begin(), cost_data.m_sculpt_ids.end());
		all_diffuse_ids.insert(cost_data.m_diffuse_ids.begin(), cost_data.m_diffuse_ids.end());

		all_normal_ids.insert(cost_data.m_normal_ids.begin(), cost_data.m_normal_ids.end());
		all_specular_ids.insert(cost_data.m_specular_ids.begin(), cost_data.m_specular_ids.end());
	}

	cost += textureCostsV2(all_sculpt_ids, all_diffuse_ids, all_normal_ids, all_specular_ids);

	// Animated Object surcharge
	const F32 animated_object_surcharge = 1000;
	if (root->isAnimatedObject())
	{
		cost += animated_object_surcharge;
	}

	return cost;
}

F32 LLObjectCostManagerImpl::getStreamingCost(U32 version, const LLVOVolume *vol)
{
	F32 streaming_cost = -1.f;
	
	if (version == 0)
	{
		version = LLObjectCostManager::instance().getCurrentCostVersion();
	}
	if (version == 1)
	{
		streaming_cost = getStreamingCostV1(vol);

#ifdef VALIDATE_COSTS
		F32 streaming_cost_legacy = vol->getStreamingCostLegacy();
		if (streaming_cost != streaming_cost_legacy)
		{
			LL_WARNS("Arctan") << "streaming cost mismatch " << streaming_cost << ", " << streaming_cost_legacy << LL_ENDL;
		}
#endif
	}
	else
	{
		LL_WARNS("Arctan") << "Unrecognized version " << version << LL_ENDL;
	}

	return streaming_cost;
};

F32 LLObjectCostManagerImpl::getStreamingCostV1(const LLVOVolume *vol)
{
	F32 radius = vol->getScale().length()*0.5f;
	F32 linkset_base_cost = 0.f;

	LLMeshCostData costs;
	if (vol->getMeshCostData(costs))
	{
		if (vol->isAnimatedObject() && vol->isRootEdit())
		{
			// Root object of an animated object has this to account for skeleton overhead.
			linkset_base_cost = ANIMATED_OBJECT_BASE_COST;
		}
		if (vol->isMesh())
		{
			if (vol->isAnimatedObject() && vol->isRiggedMesh())
			{
				return linkset_base_cost + costs.getTriangleBasedStreamingCost();
			}
			else
			{
				return linkset_base_cost + costs.getRadiusBasedStreamingCost(radius);
			}
		}
		else
		{
			return linkset_base_cost + costs.getRadiusBasedStreamingCost(radius);
		}
	}
	else
	{
		return 0.f;
	}
}

F32 LLObjectCostManagerImpl::getStreamingCostV2(const LLVOVolume *vol)
{
	return getStreamingCostV1(vol);
}

F32 LLObjectCostManagerImpl::getRenderCost(U32 version, const LLVOVolume *vol)
{
	F32 render_cost = -1.f;
	
	if (version == 0)
	{
		version = LLObjectCostManager::instance().getCurrentCostVersion();
	}
	if (version == 1)
	{

		render_cost = getRenderCostV1(vol);

#ifdef VALIDATE_COSTS
		LLVOVolume::texture_cost_t textures, material_textures;
		U32 render_cost_legacy = vol->getRenderCostLegacy(textures, material_textures);
		if ((U32) render_cost != render_cost_legacy)
		{
			LL_WARNS("Arctan") << "render cost mismatch " << render_cost << ", " << render_cost_legacy << LL_ENDL;
		}
#endif
	}
	else if (version == 2)
	{
		render_cost = getRenderCostV2(vol);
	}
	else
	{
		LL_WARNS("Arctan") << "Unrecognized version " << version << LL_ENDL;
	}

	return render_cost;
};

F32 LLObjectCostManagerImpl::getRenderCostV1(const LLVOVolume *vol)
{
	LLPrimCostData cost_data;
	getPrimCostData(vol, cost_data);

	// Charge for effective triangles
	F32 cost = triangleCostsV1(cost_data);

	// Material textures not included in V1 costs
	cost += textureCostsV1(cost_data.m_sculpt_ids);
	cost += textureCostsV1(cost_data.m_diffuse_ids);
	
	return cost;
}

F32 LLObjectCostManagerImpl::getRenderCostV2(const LLVOVolume *vol)
{
	LLPrimCostData cost_data;
	getPrimCostData(vol, cost_data);

	// Charge for effective triangles
	F32 cost = triangleCostsV2(cost_data);

	// Material textures not included in V1 costs
	cost += textureCostsV2(cost_data.m_sculpt_ids);
	cost += textureCostsV2(cost_data.m_diffuse_ids);
	
	return cost;
}

// Accumulate data for a single prim.
void LLObjectCostManagerImpl::getPrimCostData(const LLVOVolume *vol, LLPrimCostData& cost_data)
{
	static const U32 ARC_PARTICLE_MAX = 2048; // default values

	const LLDrawable* drawablep = vol->mDrawable;
	U32 num_faces = 0;
	if (drawablep)
	{
		num_faces = drawablep->getNumFaces();
	}

	bool has_volume = (vol->getVolume() != NULL);

	cost_data.m_num_triangles_v1 = 0;
	cost_data.m_num_triangles_v2 = 0;
	
	if (has_volume)
	{
        LLMeshCostData costs;
		if (vol->getMeshCostData(costs))
		{
            if (vol->isAnimatedObject() && vol->isRiggedMesh())
            {
                // Scaling here is to make animated object vs
                // non-animated object ARC proportional to the
                // corresponding calculations for streaming cost.
                cost_data.m_num_triangles_v1 = (ANIMATED_OBJECT_COST_PER_KTRI * 0.001 * costs.getEstTrisForStreamingCost())/0.06;
            }
            else
            {
                F32 radius = vol->getScale().length()*0.5f;
                cost_data.m_num_triangles_v1 = costs.getRadiusWeightedTris(radius);
            }
		}

		cost_data.m_is_animated_object = vol->isAnimatedObject();
		cost_data.m_is_root_edit = vol->isRootEdit();
	}

	if (cost_data.m_num_triangles_v1 <= 0)
	{
		// Fallback for non-mesh prims
		cost_data.m_num_triangles_v1 = 4;
	}

	if (vol->isSculpted())
	{
		if (vol->isMesh())
		{
			cost_data.m_mesh_vols++;
			if (vol->isRiggedMesh())
			{
				cost_data.m_weighted_mesh_vols++;
			}
		}
		else
		{
			// Actual sculpty, capture its id
			const LLSculptParams *sculpt_params = (LLSculptParams *) vol->getParameterEntry(LLNetworkData::PARAMS_SCULPT);
			LLUUID sculpt_id = sculpt_params->getSculptTexture();
			cost_data.m_sculpt_ids.insert(sculpt_id);
			cost_data.m_sculpt_vols++;
		}
	}

	if (vol->isFlexible())
	{
		cost_data.m_flexi_vols++;
	}


	if (vol->isParticleSource())
	{
		cost_data.m_particle_source_vols++;
	}

	if (vol->getIsLight())
	{
		cost_data.m_produces_light_vols++;
	}

	U32 materials_faces_this_vol = 0;

	for (S32 i = 0; i < num_faces; ++i)
	{
		const LLFace* face = drawablep->getFace(i);
		if (!face) continue;
		const LLTextureEntry* te = face->getTextureEntry();
		const LLViewerTexture* img = face->getTexture();

        LLMaterial* mat = NULL;
        if (te)
        {
            mat = te->getMaterialParams();
        }
        if (mat)
        {
			materials_faces_this_vol++;
            if (mat->getNormalID().notNull())
            {
                cost_data.m_normalmap_faces++;
                LLUUID normal_id = mat->getNormalID();
				cost_data.m_normal_ids.insert(normal_id);
            }
            if (mat->getSpecularID().notNull())
            {
                cost_data.m_specmap_faces++;
                LLUUID spec_id = mat->getSpecularID();
				cost_data.m_specular_ids.insert(spec_id);
            }
        }
		if (img)
		{
			cost_data.m_diffuse_ids.insert(img->getID());
		}

		if (face->getPoolType() == LLDrawPool::POOL_ALPHA)
		{
			cost_data.m_alpha_faces++;
		}
        else if ((face->getPoolType() == LLDrawPool::POOL_ALPHA_MASK) || (face->getPoolType() == LLDrawPool::POOL_FULLBRIGHT_ALPHA_MASK))
        {
            cost_data.m_alpha_mask_faces++;
        }
        else if (face->getPoolType() == LLDrawPool::POOL_FULLBRIGHT)
        {
			// FIXME ARC: this only gets hit for full bright faces
			// with some graphics quality settings (e.g. on
			// low). Doesn't make sense since render cost should only
			// be a function of the content.
            cost_data.m_full_bright_faces++;
        }
		else if (img && img->getPrimaryFormat() == GL_ALPHA)
		{
			// ARC FIXME what is this really measuring?
			cost_data.m_invisi_faces = 1;
		}
		if (face->hasMedia())
		{
			cost_data.m_media_faces++;
		}

		if (te)
		{
            if (te->getBumpmap())
			{
				cost_data.m_bumpmap_faces++;
			}
            if (te->getBumpmap() || te->getBumpShiny() || te->getBumpShinyFullbright())
			{
				cost_data.m_bump_any_faces++;
			}
            if (te->getShiny())
			{
				cost_data.m_shiny_faces++;
			}
            if (te->getShiny() || te->getBumpShiny() || te->getBumpShinyFullbright())
			{
				cost_data.m_shiny_any_faces++;
			}
            if (te->getFullbright() || te->getBumpShinyFullbright())
            {
                cost_data.m_full_bright_faces++;
            }
			if (te->getGlow() > 0.f)
			{
				cost_data.m_glow_faces++;
			}
			if (face->mTextureMatrix != NULL)
			{
				cost_data.m_animtex_faces++;
			}
			if (te->getTexGen())
			{
				cost_data.m_planar_faces++;
			}
		}
	}

	if (materials_faces_this_vol > 0)
	{
		cost_data.m_materials_vols++;
		cost_data.m_materials_faces += materials_faces_this_vol;
	}

	if (vol->isParticleSource())
	{
		const LLPartSysData *part_sys_data = &(vol->mPartSourcep->mPartSysData);
		const LLPartData *part_data = &(part_sys_data->mPartData);
		U32 num_particles = (U32)(part_sys_data->mBurstPartCount * llceil( part_data->mMaxAge / part_sys_data->mBurstRate));
		num_particles = num_particles > ARC_PARTICLE_MAX ? ARC_PARTICLE_MAX : num_particles;
		F32 part_size = (llmax(part_data->mStartScale[0], part_data->mEndScale[0]) + llmax(part_data->mStartScale[1], part_data->mEndScale[1])) / 2.f;

		cost_data.m_num_particles += num_particles;
		cost_data.m_part_size += part_size;
		// ARC - how do we use this info? how do we aggregate it across multiple prims?
	}

    cost_data.m_triangle_count = vol->getTriangleCount();

    cost_data.m_triangle_count_lowest = vol->getLODTriangleCount(LLModel::LOD_IMPOSTOR);
    cost_data.m_triangle_count_low = vol->getLODTriangleCount(LLModel::LOD_LOW);
    cost_data.m_triangle_count_medium = vol->getLODTriangleCount(LLModel::LOD_MEDIUM);
    cost_data.m_triangle_count_high = vol->getLODTriangleCount(LLModel::LOD_HIGH);

	std::vector<F32> triangle_counts_by_lod(4);
	triangle_counts_by_lod[0] = cost_data.m_triangle_count_lowest;
	triangle_counts_by_lod[1] = cost_data.m_triangle_count_low;
	triangle_counts_by_lod[2] = cost_data.m_triangle_count_medium;
	triangle_counts_by_lod[3] = cost_data.m_triangle_count_high;
	cost_data.m_actual_triangles_charged = LLMeshCostData::getChargedTriangleCount(triangle_counts_by_lod);

	// ARC TODO: here using actual tris, which will be a bit different than the estimation-based formula.
	cost_data.m_num_triangles_v2 = cost_data.m_actual_triangles_charged;
}

U32 LLObjectCostManagerImpl::textureCostsV1(const texture_ids_t& ids)
{
	U32 cost = 0;

    static const U32 ARC_TEXTURE_COST    = 16; // multiplier for texture resolution - performance tested

	// FIXME ARC Media faces do not give the right dimensions. Old
	// code uses face texture directly, right value. Here we look up
	// the corresponding fetched texture, doesn't work (get 0x0 texture).
	for (texture_ids_t::const_iterator it = ids.begin(); it != ids.end(); ++it)
	{
		const LLUUID& id = *it;
		LLViewerFetchedTexture *texture = LLViewerTextureManager::getFetchedTexture(id);
		U32 texture_cost = 0;
		if (texture)
		{
			texture_cost = 256 + (S32)(ARC_TEXTURE_COST * (texture->getFullHeight() / 128.f + texture->getFullWidth() / 128.f));
		}
		else
		{
			texture_cost = 1;
		}
		LL_DEBUGS("ARCdetail") << "texture " << id << " cost " << texture_cost << LL_ENDL;
		cost += texture_cost;
	}

	return cost;
}

U32 LLObjectCostManagerImpl::textureCostsV2(const texture_ids_t& all_sculpt_ids, const texture_ids_t& all_diffuse_ids,
											const texture_ids_t& all_normal_ids, const texture_ids_t& all_specular_ids)
{
	U32 cost = 0;

	// Material textures not included in V1 costs
	cost += textureCostsV2(all_sculpt_ids);
	cost += textureCostsV2(all_diffuse_ids);

	// FIXME ARC including material textures here to force ARC to change for logging purposes. This differs from V1
	cost += textureCostsV2(all_normal_ids, 2);
	cost += textureCostsV2(all_specular_ids, 3);

	return cost;
}

U32 LLObjectCostManagerImpl::textureCostsV2(const texture_ids_t& ids, U32 multiplier)
{
	U32 cost = 0;

    static const U32 ARC_TEXTURE_COST    = 16; // multiplier for texture resolution - performance tested

	// FIXME ARC Media faces do not give the right dimensions. Old
	// code uses face texture directly, right value. Here we look up
	// the corresponding fetched texture, doesn't work (get 0x0 texture).
	for (texture_ids_t::const_iterator it = ids.begin(); it != ids.end(); ++it)
	{
		const LLUUID& id = *it;
		LLViewerFetchedTexture *texture = LLViewerTextureManager::getFetchedTexture(id);
		U32 texture_cost = 0;
		if (texture)
		{
			texture_cost = 256 + (S32)(ARC_TEXTURE_COST * (texture->getFullHeight() / 128.f + texture->getFullWidth() / 128.f));
		}
		else
		{
			texture_cost = 1;
		}
		// FIXME ARC another hack to force RC change
		texture_cost += 5 - texture->getDiscardLevel();
		LL_DEBUGS("ARCdetail") << "texture " << id << " cost " << texture_cost << LL_ENDL;
		cost += texture_cost;
	}

	// FIXME ARC multiplier is just a hack to make costs more likely to differ when number/size of textures is the same. Should always be 1.
	cost *= multiplier;

	return cost;
}

F32 LLObjectCostManagerImpl::triangleCostsV1(LLPrimCostData& cost_data)
{
    /*****************************************************************
     * This calculation should not be modified by third party viewers,
     * since it is used to limit rendering and should be uniform for
     * everyone. If you have suggested improvements, submit them to
     * the official viewer for consideration.
     *****************************************************************/

	// per-prim costs
	static const U32 ARC_PARTICLE_COST = 1; // determined experimentally
	//static const U32 ARC_PARTICLE_MAX = 2048; // default values
	//static const U32 ARC_TEXTURE_COST = 16; // multiplier for texture resolution - performance tested
	static const U32 ARC_LIGHT_COST = 500; // static cost for light-producing prims 
	static const U32 ARC_MEDIA_FACE_COST = 1500; // static cost per media-enabled face 


	// per-prim multipliers
	static const F32 ARC_GLOW_MULT = 1.5f; // tested based on performance
	static const F32 ARC_BUMP_MULT = 1.25f; // tested based on performance
	static const F32 ARC_FLEXI_MULT = 5; // tested based on performance
	static const F32 ARC_SHINY_MULT = 1.6f; // tested based on performance
	static const F32 ARC_INVISI_COST = 1.2f; // tested based on performance
	static const F32 ARC_WEIGHTED_MESH = 1.2f; // tested based on performance

	static const F32 ARC_PLANAR_COST = 1.0f; // tested based on performance to have negligible impact
	static const F32 ARC_ANIM_TEX_COST = 4.f; // tested based on performance
	static const F32 ARC_ALPHA_COST = 4.f; // 4x max - based on performance

	F32 cost = 0;

	U32 invisi = (cost_data.m_invisi_faces > 0);
	U32 shiny = (cost_data.m_shiny_faces > 0);
	U32 glow = (cost_data.m_glow_faces > 0);
	U32 alpha = (cost_data.m_alpha_faces > 0);
	U32 flexi = (cost_data.m_flexi_vols > 0);
	U32 animtex = (cost_data.m_animtex_faces > 0);
	U32 particles = (cost_data.m_particle_source_vols > 0);
	U32 bump = (cost_data.m_bumpmap_faces > 0);
	U32 planar = (cost_data.m_planar_faces > 0);
	U32 weighted_mesh = (cost_data.m_weighted_mesh_vols > 0);
	U32 produces_light = (cost_data.m_produces_light_vols > 0);
	U32 media_faces = cost_data.m_media_faces;

	U32 num_triangles = cost_data.m_num_triangles_v1;
	
	// cost currently has the "base" cost of 1 point per 15 triangles, min 2.
	cost = num_triangles  * 5.f;
	cost = cost < 2.f ? 2.f : cost;

	// multiply by per-face modifiers
	if (planar)
	{
		cost *= planar * ARC_PLANAR_COST;
	}

	if (animtex)
	{
		cost *= animtex * ARC_ANIM_TEX_COST;
	}

	if (alpha)
	{
		cost *= alpha * ARC_ALPHA_COST;
	}

	if(invisi)
	{
		cost *= invisi * ARC_INVISI_COST;
	}

	if (glow)
	{
		cost *= glow * ARC_GLOW_MULT;
	}

	if (bump)
	{
		cost *= bump * ARC_BUMP_MULT;
	}

	if (shiny)
	{
		cost *= shiny * ARC_SHINY_MULT;
	}


	// multiply cost by multipliers
	if (weighted_mesh)
	{
		cost *= weighted_mesh * ARC_WEIGHTED_MESH;
	}

	if (flexi)
	{
		cost *= flexi * ARC_FLEXI_MULT;
	}


	// add additional costs
	if (particles)
	{
		cost += cost_data.m_num_particles * cost_data.m_part_size * ARC_PARTICLE_COST;
	}

	if (produces_light)
	{
		cost += ARC_LIGHT_COST;
	}

	if (media_faces)
	{
		cost += media_faces * ARC_MEDIA_FACE_COST;
	}

    // Streaming cost for animated objects includes a fixed cost
    // per linkset. Add a corresponding charge here translated into
    // triangles, but not weighted by any graphics properties.
    if (cost_data.m_is_animated_object && cost_data.m_is_root_edit) 
    {
        cost += (ANIMATED_OBJECT_BASE_COST/0.06) * 5.0f;
    }

	return cost;
}
		   
// FIXME ARC - initially same as V1, eventually costs will differ here.
F32 LLObjectCostManagerImpl::triangleCostsV2(LLPrimCostData& cost_data)
{
    /*****************************************************************
     * This calculation should not be modified by third party viewers,
     * since it is used to limit rendering and should be uniform for
     * everyone. If you have suggested improvements, submit them to
     * the official viewer for consideration.
     *****************************************************************/

	// per-prim costs
	static const U32 ARC_PARTICLE_COST = 1; // determined experimentally
	//static const U32 ARC_PARTICLE_MAX = 2048; // default values
	//static const U32 ARC_TEXTURE_COST = 16; // multiplier for texture resolution - performance tested
	static const U32 ARC_LIGHT_COST = 500; // static cost for light-producing prims 
	static const U32 ARC_MEDIA_FACE_COST = 1500; // static cost per media-enabled face 


	// per-prim multipliers
	static const F32 ARC_GLOW_MULT = 1.5f; // tested based on performance
	static const F32 ARC_BUMP_MULT = 1.25f; // tested based on performance
	static const F32 ARC_FLEXI_MULT = 5; // tested based on performance
	static const F32 ARC_SHINY_MULT = 1.6f; // tested based on performance
	static const F32 ARC_INVISI_COST = 1.2f; // tested based on performance
	static const F32 ARC_WEIGHTED_MESH = 1.2f; // ARC TODO: review static vs rigged mesh cost

	static const F32 ARC_PLANAR_COST = 1.0f; // tested based on performance to have negligible impact
	static const F32 ARC_ANIM_TEX_COST = 4.f; // tested based on performance
	static const F32 ARC_ALPHA_COST = 4.f; // 4x max - based on performance

	// ARC TODO: value TBD by testing, initially using value from v1 Animated Object ARC.
	//static const F32 ARC_V2_TRIANGLE_WEIGHT = ANIMATED_OBJECT_BASE_COST * 0.001 / 0.06;
	static const F32 ARC_V2_TRIANGLE_WEIGHT = 1.0; // What if we just count tris?
	
	F32 cost = 0;

	U32 invisi = (cost_data.m_invisi_faces > 0);
	U32 shiny = (cost_data.m_shiny_faces > 0);
	U32 glow = (cost_data.m_glow_faces > 0);
	U32 alpha = (cost_data.m_alpha_faces > 0);
	U32 flexi = (cost_data.m_flexi_vols > 0);
	U32 animtex = (cost_data.m_animtex_faces > 0);
	U32 particles = (cost_data.m_particle_source_vols > 0);
	U32 bump = (cost_data.m_bumpmap_faces > 0);
	U32 planar = (cost_data.m_planar_faces > 0);
	U32 weighted_mesh = (cost_data.m_weighted_mesh_vols > 0);
	U32 produces_light = (cost_data.m_produces_light_vols > 0);
	U32 media_faces = cost_data.m_media_faces;

	U32 num_triangles = cost_data.m_num_triangles_v2;

	// ARC TODO: figure out any weighting or clamping
	cost = ARC_V2_TRIANGLE_WEIGHT * num_triangles; 

	// multiply by per-face modifiers
	if (planar)
	{
		cost *= planar * ARC_PLANAR_COST;
	}

	if (animtex)
	{
		cost *= animtex * ARC_ANIM_TEX_COST;
	}

	if (alpha)
	{
		cost *= alpha * ARC_ALPHA_COST;
	}

	if(invisi)
	{
		cost *= invisi * ARC_INVISI_COST;
	}

	if (glow)
	{
		cost *= glow * ARC_GLOW_MULT;
	}

	if (bump)
	{
		cost *= bump * ARC_BUMP_MULT;
	}

	if (shiny)
	{
		cost *= shiny * ARC_SHINY_MULT;
	}


	// multiply cost by multipliers
	if (weighted_mesh)
	{
		cost *= weighted_mesh * ARC_WEIGHTED_MESH;
	}

	if (flexi)
	{
		cost *= flexi * ARC_FLEXI_MULT;
	}


	// add additional costs
	if (particles)
	{
		cost += cost_data.m_num_particles * cost_data.m_part_size * ARC_PARTICLE_COST;
	}

	if (produces_light)
	{
		cost += ARC_LIGHT_COST;
	}

	if (media_faces)
	{
		cost += media_faces * ARC_MEDIA_FACE_COST;
	}

	// FIXME ARC - include this?
#if 0	
    // Streaming cost for animated objects includes a fixed cost
    // per linkset. Add a corresponding charge here translated into
    // triangles, but not weighted by any graphics properties.
    if (cost_data.m_is_animated_object && cost_data.m_is_root_edit) 
    {
        cost += (ANIMATED_OBJECT_BASE_COST/0.06) * 5.0f;
    }
#endif

	return cost;
}
		   
LLObjectCostManager::LLObjectCostManager()
{
	m_impl = new LLObjectCostManagerImpl;
}

LLObjectCostManager::~LLObjectCostManager()
{
	delete(m_impl);
	m_impl = NULL;
}

U32 LLObjectCostManager::getCurrentCostVersion()
{
	return 2;
}

F32 LLObjectCostManager::getStreamingCost(U32 version, const LLVOVolume *vol)
{
	F32 streaming_cost = m_impl->getStreamingCost(version, vol);
	return streaming_cost;
}

F32 LLObjectCostManager::getRenderCost(U32 version, const LLVOVolume *vol)
{
	F32 render_cost = m_impl->getRenderCost(version, vol);
	return render_cost;
}

F32 LLObjectCostManager::getRenderCostLinkset(U32 version, const LLViewerObject *root)
{
	F32 render_cost = 0.f;
	if (!root->isRootEdit())
	{
		LL_WARNS("Arctan") << "called with non-root object" << LL_ENDL;
	}
	else
	{
		render_cost = m_impl->getRenderCostLinkset(version, root);
	}
	return render_cost;
}

LLSD texture_as_llsd(const LLUUID& id)
{
	LLSD texture_sd = LLSD::emptyMap();
	texture_sd["id"] = id;
	LLViewerFetchedTexture *texture = LLViewerTextureManager::getFetchedTexture(id);
	if (texture)
	{
		texture_sd["width"] = (LLSD::Integer) texture->getFullWidth();
		texture_sd["height"] = (LLSD::Integer) texture->getFullHeight();
		texture_sd["format"] = (LLSD::Integer) texture->getPrimaryFormat();
		texture_sd["components"] = (LLSD::Integer) texture->getComponents();
		texture_sd["discard"] = (LLSD::Integer) texture->getDiscardLevel();
	}
	else
	{
		texture_sd["width"] = (LLSD::Integer) -1;
		texture_sd["height"] = (LLSD::Integer) -1;
		texture_sd["format"] = (LLSD::Integer) -1;
		texture_sd["components"] = (LLSD::Integer) -1;
		texture_sd["discard"] = (LLSD::Integer) -1;
	}
	return texture_sd;
}

LLSD textures_as_llsd(texture_ids_t& ids)
{
	LLSD array_sd = LLSD::emptyArray();
	for (texture_ids_t::const_iterator it = ids.begin(); it != ids.end(); ++it)
	{
		const LLUUID& id = *it;
		array_sd.append(texture_as_llsd(id));
	}
	return array_sd;

}

LLSD LLObjectCostManager::getFrameDataPrim(const LLVOVolume *vol)
{
	LLSD sd;
	LLPrimCostData cost_data;

	m_impl->getPrimCostData(vol, cost_data);
	cost_data.asLLSD(sd);
	return sd;
}

LLSD LLObjectCostManager::getFrameDataLinkset(const LLVOVolume *vol)
{
	const_vol_vec_t volumes;
	get_volumes_for_linkset(vol, volumes);

	LLSD sd;

	texture_ids_t all_sculpt_ids, all_diffuse_ids, all_normal_ids, all_specular_ids;

	if (volumes.size()>0)
	{
		sd["id"] = (LLSD::UUID) vol->getID();
		sd["prims"] = LLSD::emptyArray();
		LLSD linkset_sd; // summary data for the linkset as a whole.

		U32 linkset_num_triangles_v2 = 0;
		F32 linkset_triangle_costs_v2 = 0;
		U32 linkset_prim_count = 0;

		for (std::vector<const LLVOVolume*>::const_iterator it = volumes.begin();
			 it != volumes.end(); ++it)
		{
			const LLVOVolume *vol = *it;

			LLPrimCostData cost_data;
			m_impl->getPrimCostData(vol, cost_data);
			LLSD prim_sd;
			cost_data.asLLSD(prim_sd);
			sd["prims"].append(prim_sd);

			// Accumulate texture ids
			all_diffuse_ids.insert(cost_data.m_diffuse_ids.begin(), cost_data.m_diffuse_ids.end());
			all_sculpt_ids.insert(cost_data.m_sculpt_ids.begin(), cost_data.m_sculpt_ids.end());
			all_normal_ids.insert(cost_data.m_normal_ids.begin(), cost_data.m_normal_ids.end());
			all_specular_ids.insert(cost_data.m_specular_ids.begin(), cost_data.m_specular_ids.end());

			// Accumulate summary info
			linkset_num_triangles_v2 += cost_data.m_num_triangles_v2;
			linkset_triangle_costs_v2 += m_impl->triangleCostsV2(cost_data);
			linkset_prim_count++;
		}
		F32 linkset_texture_costs_v2 = m_impl->textureCostsV2(all_diffuse_ids, all_sculpt_ids, all_normal_ids, all_specular_ids);

		linkset_sd["num_triangles_v2"] = LLSD::Integer(linkset_num_triangles_v2);
		linkset_sd["triangle_costs_v2"] = linkset_triangle_costs_v2;
		linkset_sd["texture_costs_v2"] = linkset_texture_costs_v2;
		linkset_sd["prim_count"] = LLSD::Integer(linkset_prim_count);
		linkset_sd["texture_count"]["diffuse"] = LLSD::Integer(all_diffuse_ids.size());
		linkset_sd["texture_count"]["sculpt"] = LLSD::Integer(all_sculpt_ids.size());
		linkset_sd["texture_count"]["normal"] = LLSD::Integer(all_normal_ids.size());
		linkset_sd["texture_count"]["specular"] = LLSD::Integer(all_specular_ids.size());

		sd["linkset_cost_summary"] = linkset_sd;
		
		sd["textures"] = LLSD::emptyMap();
		sd["textures"]["diffuse"] = textures_as_llsd(all_diffuse_ids);
		sd["textures"]["sculpt"] = textures_as_llsd(all_sculpt_ids);
		sd["textures"]["specular"] = textures_as_llsd(all_normal_ids);
		sd["textures"]["normal"] = textures_as_llsd(all_specular_ids);
	}

	return sd;
}
LLSD LLObjectCostManager::getFrameDataAvatar(const LLVOAvatar *av)
{
    LLSD av_sd;
    av_sd["Name"] = (LLSD::String) av->getFullname();
    av_sd["Self"] = (LLSD::Boolean) av->isSelf();
    av_sd["UUID"] = (LLSD::UUID) av->getID();
	std::string viz_string = LLVOAvatar::rezStatusToString(av->getRezzedStatus());
    av_sd["RezStatus"] = (LLSD::String) viz_string;
    av_sd["ARCCalculated"] = (LLSD::Integer) av->getVisualComplexity(); 
    av_sd["ARCCalculated1"] = (LLSD::Integer) av->getVisualComplexity(1);
    av_sd["ARCCalculated2"] = (LLSD::Integer) av->getVisualComplexity(2);
	av_sd["ARCCalculatedOld"] = (LLSD::Integer) av->getVisualComplexity(99); // FIXME ARC remove when old code path goes away
    av_sd["ARCReported"] = (LLSD::Integer) av->getReportedVisualComplexity();
    av_sd["AttachmentSurfaceArea"] = (LLSD::Real) av->getAttachmentSurfaceArea();
    if (av->isSelf())
    {
        std::string outfit_name;
        if (LLAppearanceMgr::instance().getBaseOutfitName(outfit_name))
        {
            av_sd["OutfitName"] = (LLSD::String) outfit_name;
        }
        else
        {
            outfit_name = LLAppearanceMgr::instance().getLastRequestedCategoryName();
            if (!outfit_name.empty())
            {
                av_sd["OutfitName"] = outfit_name;
            }
            else
            {
                av_sd["OutfitName"] = "Unknown Outfit";
            }
        }
    }
    LLSD av_attachments = LLSD::emptyArray();

    LLVOVolume::texture_cost_t textures;
    LLVOVolume::texture_cost_t material_textures;

	U32 av_num_triangles_v2 = 0;
	F32 av_triangle_costs_v2 = 0;
	F32 av_texture_costs_v2 = 0;
	U32 av_attachment_count = 0;


    // For each attached volume (top level or child), generate an LLSD record 
    for (LLVOAvatar::attachment_map_t::const_iterator attachment_point = av->mAttachmentPoints.begin(); 
         attachment_point != av->mAttachmentPoints.end();
         ++attachment_point)
    {
        LLViewerJointAttachment* attachment = attachment_point->second;
        for (LLViewerJointAttachment::attachedobjs_vec_t::iterator attachment_iter = attachment->mAttachedObjects.begin();
             attachment_iter != attachment->mAttachedObjects.end();
             ++attachment_iter)
        {
            const LLViewerObject* attached_object = (*attachment_iter);
            if (attached_object && !attached_object->isHUDAttachment())
            {
				LLSD attachment_sd = attached_object->getFrameDataLinkset();
				av_attachments.append(attachment_sd);

				av_num_triangles_v2 += attachment_sd["linkset_cost_summary"]["num_triangles_v2"].asInteger();
				av_triangle_costs_v2 += attachment_sd["linkset_cost_summary"]["triangle_costs_v2"].asReal();
				av_texture_costs_v2 += attachment_sd["linkset_cost_summary"]["texture_costs_v2"].asReal();
				av_attachment_count++;
            }
        }

    }
    av_sd["Attachments"] = av_attachments;

	LLSD summary_sd;
	summary_sd["num_triangles_v2"] = LLSD::Integer(av_num_triangles_v2);
	summary_sd["triangle_costs_v2"] = LLSD::Real(av_triangle_costs_v2);
	summary_sd["texture_costs_v2"] = LLSD::Real(av_texture_costs_v2);
	summary_sd["attachment_count"] = LLSD::Integer(av_attachment_count);

	av_sd["av_cost_summary"] = summary_sd;
	
    return av_sd;
}
