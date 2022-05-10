package com.inasweaterpoorlyknit.learnopengl_androidport.ui

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.inasweaterpoorlyknit.learnopengl_androidport.R

class ProgramListItemData(val imageResourceId: Int,
                          val textResourceId: Int)

class ProgramListItemAdapter(private val programListItemsData: Array<ProgramListItemData>,
                             private val onClickCallback: (imageResourceId: Int) -> Unit) :
    RecyclerView.Adapter<ProgramListItemAdapter.ProgramListItemViewHolder>() {

    class ProgramListItemViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        var programSampleImage: ImageView = itemView.findViewById(R.id.program_sample_image)
        var programTitle: TextView = itemView.findViewById(R.id.program_title_text)
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ProgramListItemViewHolder {
        val view = LayoutInflater.from(parent.context).inflate(R.layout.program_recycler_item, parent, false)
        return ProgramListItemViewHolder(view)
    }

    override fun onBindViewHolder(viewHolder: ProgramListItemViewHolder, position: Int) {
        viewHolder.programSampleImage.setImageResource(programListItemsData[position].imageResourceId)
        viewHolder.programTitle.setText(programListItemsData[position].textResourceId)
        viewHolder.itemView.setOnClickListener{ onClickCallback(programListItemsData[position].imageResourceId) }
    }

    override fun getItemCount() = programListItemsData.size
}
