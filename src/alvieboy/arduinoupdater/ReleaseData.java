package arduinoupdater;

import java.util.*;
import java.net.*;
import java.io.*;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;

public class ReleaseData
{
    public class Branch {
        public String name;
        public String description;
        public Release leaf;
    };

    public class Resource {
        public int id;
        public String sha;
        public int size;
    };

    public class ReleaseFile {
        String target;
        int rsid;
        boolean executable;
    }

    public class MatchResult {
        public MatchResult() {
            newFiles = new ReleaseFileList();
            deprecatedFiles = new ReleaseFileList();
            replacedFiles = new ReleaseFileList();
            keptFiles = new ReleaseFileList();
        }
        ReleaseFileList newFiles;
        ReleaseFileList deprecatedFiles;
        ReleaseFileList replacedFiles;
        ReleaseFileList keptFiles;
    }

    public class ReleaseFileList extends ArrayList<ReleaseFile> {

        public boolean existsTarget(String target) {
            Iterator itr = this.iterator();
            while(itr.hasNext()) {
                ReleaseFile f = (ReleaseFile)itr.next();
                if (f.target.equals(target))
                    return true;
            }
            return false;
        }
        public void removeByTarget(String target) {
            Iterator itr = this.iterator();
            while(itr.hasNext()) {
                ReleaseFile f = (ReleaseFile)itr.next();
                if (f.target.equals(target)) {
                    this.remove(f);
                    return;
                }
            }
        }
    }

    public class Release {
        Release parent;
        String version;
        String basedir;
        ReleaseFileList files;

        public ReleaseFileList getFiles() {
            ReleaseFileList myFiles=null;
            if (null!=parent) {
                myFiles=parent.getFiles();
            }
            if (null==myFiles)
                myFiles = new ReleaseFileList();
            appendMine(myFiles);
            return myFiles;
        }
        private void appendMine(ReleaseFileList allfiles) {
            if (null==files) {
                /* Odd... */
                return;
            }
            for (ReleaseFile f: files) {
                allfiles.removeByTarget(f.target);
                if (f.rsid > 0) {
                    allfiles.add(f);
                }
            }
        }
        public void showFiles()
        {
            ReleaseFileList files = getFiles();
            for (ReleaseFile f: files) {
                Debugger.debug("File: " + m_resources.get(f.rsid).sha + " " +f.target);
            }
        }
    };

    public class DownloadError extends Exception
    {
    };

    public class MalformedXMLException extends Exception {
        MalformedXMLException() {super();}
        MalformedXMLException(String s) { super(s); }
    };

    public ReleaseData() {
        m_branches=new ArrayList<Branch>();
        m_releases=new ArrayList<Release>();
    }

    static Element getFirstChild(Element base, String name)
    {
        for(Node n=base.getFirstChild(); n!=null; n=n.getNextSibling()){
            if (n.getNodeType()==Node.ELEMENT_NODE) {
                if (((Element)n).getTagName().equals(name))
                    return (Element)n;
            }
        }
        return null;
    }

    public Release findReleaseByVersion(String version)
    {
        for (Release r: m_releases) {
            if (r.version.equals(version))
                return r;
        }
        return null;
    }

    public void parse(Document doc) throws MalformedXMLException {
        int i;
        m_doc=doc;
        m_root = doc.getDocumentElement();
        if (m_root.getTagName() != "UpdateList")
            throw new MalformedXMLException();
        Element configNode = getFirstChild(m_root, "Configuration");
        if (null==configNode)
            throw new MalformedXMLException();
        Element releaseNode = getFirstChild(m_root,"Releases");
        if (null==releaseNode)
            throw new MalformedXMLException();
        Element resourceNode = getFirstChild(m_root,"Resources");
        if (null==resourceNode)
            throw new MalformedXMLException();
        Element branchesNode = getFirstChild(configNode,"Branches");
        if (null==branchesNode)
            throw new MalformedXMLException();

        Element URL = getFirstChild(configNode, "URL");
        if (null!=URL) {
            m_base = URL.getAttribute("base");
        }


        NodeList resources = resourceNode.getElementsByTagName("Resources");
        for (i=0; i<resources.getLength();i++){
            Element e = (Element)resources.item(i);
            Resource r = new Resource();

            r.id =Integer.parseInt( e.getAttribute("id"));
            r.size =Integer.parseInt( e.getAttribute("size"));
            r.sha = e.getAttribute("sha");
            m_resources.put(new Integer(r.id),r);
        }

        Debugger.debug("Iterating through branches");

        NodeList releases = releaseNode.getElementsByTagName("Release");
        for (i=0; i<releases.getLength();i++){
            Element e = (Element)releases.item(i);
            Release r = new Release();
            r.version = e.getAttribute("version");
            r.basedir = e.getAttribute("basedir");
            Debugger.debug("New release, version " + r.version);
            String parent = e.getAttribute("parent");
            if (parent!=null && parent.length()>0) {
                r.parent = findReleaseByVersion(parent);
                if (null==r.parent) {
                    Debugger.debug("Cannot locate release "+parent);
                    throw new MalformedXMLException();
                }
            }
            r.files = new ReleaseFileList();
            m_releases.add(r);
            /* Add files */

            NodeList filelist = e.getElementsByTagName("File");
            Debugger.debug("Release has " + filelist.getLength() + " files");
            for (int j=0; j<filelist.getLength(); j++) {
                Element f = (Element)filelist.item(j);
                ReleaseFile rf = new ReleaseFile();
                rf.target =  f.getAttribute("target");
                rf.rsid =  Integer.parseInt(f.getAttribute("rsid"));
                rf.executable=false;
                try {
                    if (f.getAttribute("exec").equals("true"))
                        rf.executable=true;
                } catch (Exception ex) {
                }
                r.files.add(rf);
            }
        }

        for (Release r: m_releases) {
            Debugger.debug("Release: "+r.version);
            r.showFiles();
        }
        NodeList branches = branchesNode.getElementsByTagName("Branch");
        for (i=0; i<branches.getLength();i++){
            Element e = (Element)branches.item(i);
            Branch b = new Branch();
            b.name = e.getAttribute("name");
            String leaf = e.getAttribute("leaf");
            b.leaf = findReleaseByVersion(leaf);
            if (null==b.leaf) {
                throw new MalformedXMLException("Branch '" + b.name + "' refers an unknown release '" +leaf+"'");
            }
            m_branches.add(b);
        }
    }

    public MatchResult compareToRelease(Release r, Map<String,String> current)
    {
        MatchResult res = new MatchResult();

        ReleaseFileList rf = r.getFiles();
        Map<String,Boolean> seenFiles = new HashMap<String,Boolean>();

        for (ReleaseFile f: rf) {
            // Debugger.debug("Locating " + f.target);
            if (current.containsKey(f.target)) {
                /* Compare */
                String sha = current.get(f.target);
                if (sha.equals(m_resources.get(f.rsid).sha)) {
                    res.keptFiles.add(f);
                } else {
                    res.replacedFiles.add(f);
                }
                seenFiles.put(f.target,true);
            } else {
                res.newFiles.add(f);
            }
        }
        /* See if there are unreferenced files */

        for (Map.Entry<String, String> entry : current.entrySet()) {
            if (! seenFiles.containsKey(entry.getKey())) {
                ReleaseFile old = new ReleaseFile();
                old.target = entry.getKey();
                res.deprecatedFiles.add(old);
            }
        }

        return res;
    }

    public void downloadRelease(ReleaseFileList fl, String basedir) throws MalformedURLException, IOException {
        URL u = new URL(m_base);
        downloadRelease(fl,u,basedir);
    }

    File tempfolder;

    public void downloadRelease(ReleaseFileList fl, URL base, String basedir) throws MalformedURLException, IOException {
        try {
            if (null==tempfolder)
                tempfolder = createTempDirectory();
        } catch (IOException e) {
            Debugger.debug("Cannot create temporary folder: " + e.getCause());
            throw e;
        }
        for (ReleaseFile f: fl) {
            Debugger.debug("Downloading "+m_resources.get(f.rsid).sha);
            try {
                String sha = m_resources.get(f.rsid).sha;
                URL src = new URL(base, basedir + "/" + sha );
                File target = new File(tempfolder, sha );
                if (target.exists()) {
                    Debugger.debug("Skipping " + sha + " cause already exists");
                } else {
                    download( target.getPath(), src);
                }
            } catch (MalformedURLException e)  {
                Debugger.debug("Malformed");
                throw e;
            } catch (IOException e) {
                Debugger.debug("IO error" + e.getCause());
                e.printStackTrace();
                return;
            }
        }
        /* Cleanup temp folder */
    }

    public void copyFile(File src, File dst) throws IOException
    {
        InputStream in = new FileInputStream(src);
        OutputStream out = new FileOutputStream(dst);

        byte[] buf = new byte[8192];
        int len;
        while ((len = in.read(buf)) > 0){
            out.write(buf, 0, len);
        }
        in.close();
        out.close();
        Debugger.debug("Created file " + dst.getPath());
    }

    public void applyDownloads(MatchResult r) throws IOException
    {
        for (ReleaseFile f: r.newFiles) {
            String[] path = f.target.split("/");
            File d=null;
            for (int i=0; i<path.length-1; i++) {
                d = new File(d, path[i]);
            }
            if (null!=d) {
                d.mkdirs();
            }
            File tf=new File(d,path[path.length-1]);
            copyFile(new File(tempfolder, m_resources.get(f.rsid).sha), tf);
            if (f.executable) {
                tf.setExecutable(true);
            }
        }
    }

    public void download(String filename, URL url) throws MalformedURLException, IOException
    {
    	BufferedInputStream in = null;
    	FileOutputStream fout = null;
    	try
    	{
            in = new BufferedInputStream(url.openStream());
            fout = new FileOutputStream(filename);

            byte data[] = new byte[8192];
            int count;
            while ((count = in.read(data, 0, 8192)) != -1)
            {
                fout.write(data, 0, count);
            }
        }
        finally
        {
            if (in != null)
                in.close();
            if (fout != null)
                fout.close();
        }
    }

    public static File createTempDirectory() throws IOException
    {
        final File temp;

        temp = File.createTempFile("temp", Long.toString(System.nanoTime()));

        if(!(temp.delete()))
        {
            throw new IOException("Could not delete temp file: " + temp.getAbsolutePath());
        }

        if(!(temp.mkdir()))
        {
            throw new IOException("Could not create temp directory: " + temp.getAbsolutePath());
        }

        return (temp);
    }

    int getNumberOfReleases()
    {
        return m_releases.size();
    }
    int getNumberOfBranches()
    {
        return m_branches.size();
    }
    private Document m_doc;
    private Element m_root;
    private String m_base;

    List<Branch> m_branches;
    List<Release> m_releases;
    HashMap<Integer,Resource> m_resources;
};
