// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

package com.shanet.alsa_control;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;

import android.app.Activity;
import android.content.ContextWrapper;
import android.os.Bundle;
import android.util.SparseBooleanArray;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;

public class SelectDialog extends Activity {
	
    private ArrayList<String> content = null;
    private File contentIOFile = null;
    private int contentType = 0;
    private ListView contentList;
    
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        setContentView(R.layout.select_dialog);
        
        contentList = (ListView) findViewById(R.id.contentList);
        Button addContentBtn = (Button) findViewById(R.id.addContentBtn);
        
        // Get the type of dialog to show (either servers or channels
        contentType = getIntent().getExtras().getInt("dialogType");
                
        // Set the text on the add content button
        addContentBtn.setText(((contentType == Constants.SERVERS) ? R.string.addServer : R.string.addChannel));
        
        // Load known content into list
        contentIOFile = new File(new ContextWrapper(this).getFilesDir().getPath() + File.separatorChar + ((contentType == Constants.SERVERS) ? Constants.SERVERS_FILE : Constants.CHANNELS_FILE));
		try {
			content = Utils.getContent(contentIOFile);
		} catch (IOException e) {
			DialogUtils.displayErrorDialog(this, R.string.readIOErrorTitle, R.string.readIOError);
		}
        
        // Set the list adapter with the content we just loaded
		contentList.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
		contentList.setAdapter(new ArrayAdapter<String>(this, android.R.layout.simple_list_item_multiple_choice, content));
		registerForContextMenu(contentList);
        
        // Add a new content item listener
        addContentBtn.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				DialogUtils.showAddContentDialog(SelectDialog.this, contentIOFile, contentType);
			}
		});
        
        // Check any content that is in the servers/channels arrays
        ArrayList<String> tmpList = (contentType == Constants.SERVERS) ? Main.serverList : Main.channelList;
        for(int i=0; i<content.size(); i++) {
        	for(int j=0; j<tmpList.size(); j++) {
        		if(content.get(i).equals(tmpList.get(j))) {
        			contentList.setItemChecked(i, true);
        			break;
        		}
        	}
        }
  	}
	
	
	@Override
	public void onPause() {
		super.onPause();
        
		// On pause, copy whatever is selected to the servers and channels arrays
		ArrayList<String> tmpList = (contentType == Constants.SERVERS) ? Main.serverList : Main.channelList;
		
		// Get the selected indices
        SparseBooleanArray checkedItems = contentList.getCheckedItemPositions();
        
        // Clear the servers/channels array to avoid duplicate entries and to avoid entries that may have been removed
        tmpList.clear();

        // Add the selected items to the servers/channels array
        for(int i=0; i<checkedItems.size(); i++) {
        	if(checkedItems.valueAt(i)) {
        		tmpList.add(contentList.getAdapter().getItem(checkedItems.keyAt(i)).toString());
        	}
        }
	}
	
	
	@SuppressWarnings("unchecked")
	public void addContent(String content) {
		((ArrayAdapter<String>)contentList.getAdapter()).add(content);
		notifiyDataSetChanged();
	}
	
	
	@SuppressWarnings("unchecked")
	public void removeContent(String content, int position) {
		// Uncheck the item before removing it
		contentList.setItemChecked(position, false);
		
		((ArrayAdapter<String>)contentList.getAdapter()).remove(content);
		notifiyDataSetChanged();
	}
	
	@SuppressWarnings("unchecked")
	public void notifiyDataSetChanged() {
		((ArrayAdapter<String>)contentList.getAdapter()).notifyDataSetChanged();
	}
	
	
	@Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
		if(v.getId() == R.id.contentList) {
			DialogUtils.showDeleteDialog(this, content.get(((AdapterView.AdapterContextMenuInfo)menuInfo).position), 
					                     ((AdapterView.AdapterContextMenuInfo)menuInfo).position, contentIOFile, contentType);
		}
	}
	
	
	@Override 
	public boolean onCreateOptionsMenu(Menu menu) {
		return Utils.onCreateOptionsMenu(this, menu);
	}

	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		return Utils.onOptionsItemSelected(this, item);
	}
}
